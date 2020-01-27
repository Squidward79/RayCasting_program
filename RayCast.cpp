#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <cstring>
#include "gmath.h"
#include "usr\include\GL\freeglut.h"


#define PI 3.141592
int Width;
int Height;
int Depth;
unsigned char *pVolData;
unsigned char *pImage;
unsigned char *pScaledImage;
GVec3 *pNormal;
GVec3 *pColor;
float *pOpacity;
int N = 3;
int d0 = 45, d1 = 100;
float alpha = 0.7;

double theta = 0;
GVec3 L(0.0,-1.0,0.0);

int shapeOpacity = 0;

// test 1
// unsigned char d0 = 70, d1 = 120;
// float alpha = 0.7;

// test 2
// unsigned char d0 = 120, d1 = 190;
// float alpha = 0.7;

// test 3
// unsigned char d0 = 190, d1 = 255;
// float alpha = 0.8;

// 콜백 함수 선언
void Render();
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);


void LoadData(char *file_name);
void ComputeNormal();
void AssignOpacity();
void ComputeColor();
void CreateImage();
int GetPixelIdx(int i, int j);
int GetVoxelIdx(int i, int j, int k);

class AlphaColor
{
public:
	//보간용 생성자
	AlphaColor(float _a, GVec3 _c) { a = _a; c = _c; }

	//복셀값 저장용 생성자
	AlphaColor(int x, int y, int z) 
	{
		int vidx = GetVoxelIdx(x, z, y);
		a = pOpacity[vidx];
		c = pColor[vidx];
	}
	float a;
	GVec3 c;
};

AlphaColor* InterpolationPos(AlphaColor A, AlphaColor B, float ratio);
void Phong(int vidx, GVec3 L, GSphere Obj);

int main(int argc, char **argv)
{
	// 볼륨 데이터 로딩
	LoadData("data\\bighead.txt");
	
	// OpenGL 초기화, 윈도우 크기 설정, 디스플레이 모드 설정
	glutInit(&argc, argv);
	glutInitWindowSize(Width * N, Depth * N);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// 윈도우 생성 및 콜백 함수 등록
	glutCreateWindow("Volume Renderer");
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	
	ComputeNormal();	// 법선벡터 계산
	AssignOpacity();	// 불투명도 할당
	ComputeColor();		// 복셀의 색상 계산
	CreateImage();		// 이미지를 생성

	
	std::cout << "KEY INFO\nD: rotate 10degree to right \nA:rotate 10degree to left" << std::endl;
	std::cout << "\nZ,X,C: change a type of transparency`s caluation" << std::endl;


	// 이벤트를 처리를 위한 무한 루프로 진입한다.
	glutMainLoop();
	delete [] pVolData;
	delete [] pNormal;
	delete [] pOpacity;
	delete [] pColor;
	delete [] pImage;
	delete [] pScaledImage;
	return 0;
}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(-1);

	if (key == 'd')
		theta += 10;
	if (key == 'a')
		theta -= 10;
	if (key == 'z')
	{
		d0 = 45;
		d1 = 100;
	}
	if (key == 'x')
	{
		d0 = 100;
		d1 = 255;
	}
	if (key == 'c')
	{
		d0 = 45;
		d1 = 255;
	}
	if (key == 'p')	alpha += 0.1;
	if (key == 'o')	alpha -= 0.1;
	
	if (key == '1') shapeOpacity = 0;
	if (key == '2') shapeOpacity = 1;
	if (key == '3') shapeOpacity = 2;
	
	if (key == 't') d0 += 10;
	if (key == 'g') d0 -= 10;

	if (key == 'y') d1 += 10;
	if (key == 'h') d1 -= 10;


	glutPostRedisplay();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Render()
{
	// 칼라 버퍼와 깊이 버퍼 지우기
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < Depth * N; ++i)
	{
		for (int j = 0; j < Width * N; ++j)
		{
			unsigned char r = pImage[(i / N * Width + j / N) * 3];
			unsigned char g = pImage[(i / N * Width + j / N) * 3 + 1];
			unsigned char b = pImage[(i / N * Width + j / N) * 3 + 2];
			pScaledImage[(i * Width * N + j) * 3] = r;
			pScaledImage[(i * Width * N + j) * 3 + 1] = g;
			pScaledImage[(i * Width * N + j) * 3 + 2] = b;
		}
	}

	// 칼라 버퍼에 Image 데이터를 직접 그린다.
	glDrawPixels(Width * N, Depth * N, GL_RGB, GL_UNSIGNED_BYTE, pScaledImage);

	// 칼라 버퍼 교환한다
	glutSwapBuffers();

	//세타값도 받음
	//printf("(f0, f1,) , alpha and theta = ? ");
	//scanf("%d%d%f%f", &d0, &d1, &alpha, &theta);

	//printf("(f0, f1,) ,alpha and rotation = ? ");
	//scanf("%d%d%f%lf", &d0, &d1, &alpha, &theta);

	AssignOpacity();
	ComputeColor();
	CreateImage();

	glutPostRedisplay();
}

int GetPixelIdx(int i, int j)
{
	return ((Depth - 1 - i) * Width + j) * 3;
}

int GetVoxelIdx(int i, int j, int k)
{
	return i * (Width * Height) + j * Width + k;
}

void LoadData(char *file_name)
{
	// 볼륨 헤더(*.txt) 파일을 읽는다.
	FILE *fp;
	fopen_s(&fp, file_name, "r");
	fscanf_s(fp, "%d%d%d", &Width, &Height, &Depth);
	char vol_file_name[256];
	fscanf(fp, "%s", vol_file_name);
	fclose(fp);

	// 현재 디렉토리를 헤더 파일이 있는 곳으로 변경한다.
	std::string file_path(file_name);
	int idx = file_path.rfind("\\");
	file_path = file_path.substr(0, idx);
	_chdir(file_path.c_str());

	// 렌더링에 필요한 배열을 할당한다.
	pVolData = new unsigned char [Depth * Height * Width];
	pNormal = new GVec3 [Depth * Height * Width];
	pColor = new GVec3 [Depth * Height * Width];
	pOpacity = new float [Depth * Height * Width];
	pImage = new unsigned char [Depth * Width * 3];
	pScaledImage = new unsigned char [Depth * N * Width * N * 3];

	// 볼륨 데이터를 바이너리 형태로 읽는다.
	fopen_s(&fp, vol_file_name, "rb");
	fread(pVolData, sizeof(unsigned char), Depth * Height * Width, fp);
	fclose(fp);	
}

void ComputeNormal()
{
	for (int i = 1; i < Depth - 1; ++i)
	{
		for (int j = 1; j < Height - 1; ++j)
		{
			for (int k = 1; k < Width - 1; ++k)
			{
				int vidx = GetVoxelIdx(i, j, k);

				float dx = 0.5 * (pVolData[GetVoxelIdx(i + 1, j, k)] - pVolData[GetVoxelIdx(i - 1, j, k)]);
				float dy = 0.5 * (pVolData[GetVoxelIdx(i, j + 1, k)] - pVolData[GetVoxelIdx(i, j - 1 , k)]);
				float dz = 0.5 * (pVolData[GetVoxelIdx(i, j, k + 1)] - pVolData[GetVoxelIdx(i, j, k -1 )]);

				pNormal[vidx].Set(-dx, -dy, -dz);
				pNormal[vidx].Normalize();
			}
		}
	}
}

void AssignOpacity()
{
	for (int i = 0; i < Depth; ++i)			
	{
		for (int j = 0; j < Height; ++j)	
		{
			for (int k = 0; k < Width; ++k)	
			{
				int vidx = GetVoxelIdx(i, j, k);
				
				// Density에 따라 Opacity값이 Linear하게 증가하고 감소함
				switch (shapeOpacity)
				{
				case 0: // 직사각형
					if (d0 <= pVolData[vidx] && pVolData[vidx] <= d1)
						pOpacity[vidx] = alpha;
					else
						pOpacity[vidx] = 0;
					break;

				case 1: //사다리꼴 (선형적 증가)
					if (d0 <= pVolData[vidx] && pVolData[vidx] <= d0 + 10)
					{
						float s_opacity = (pVolData[vidx] - (float)d0) / 10.0;
						pOpacity[vidx] = alpha * s_opacity; 
					}
					else if (d0 + 10 < pVolData[vidx] && pVolData[vidx] <= d1 - 10)
					{
						pOpacity[vidx] = alpha;
					}
					else if (d1 - 10 < pVolData[vidx] && pVolData[vidx] <= d1)
					{
						float s_opacity = (pVolData[vidx] - ((float)d1 -10.0)) / 10.0;
						pOpacity[vidx] = alpha * s_opacity;
					}
					else
						pOpacity[vidx] = 0;

					break;

				case 2:
					if (d0 <= pVolData[vidx] && pVolData[vidx] <= d1)
					{
						float d = (float)(d1 + d0) / 2.0;
						float a = alpha / (SQR(d) - d*(d1+d0) + d1*d0);
						pOpacity[vidx] = a * (SQR(pVolData[vidx]) - pVolData[vidx] * (d1 + d0) + d1 * d0);
					}
					else
						pOpacity[vidx] = 0;
					break;

				default :
						break;
				}
			}
		}
	}
}

void ComputeColor()
{
	GSphere Skin;
	Skin.Kd.Set(0.7490, 1.0, 0);
	Skin.Ks.Set(1.0, 1.0, 1.0);

	GSphere Bone;
	Bone.Kd.Set(1.0, 0.8431, 0.0);
	Bone.Ks.Set(1.0, 1.0, 1.0);

	GSphere ETC;
	ETC.Kd.Set(1.0, 1.0, 1.0);
	ETC.Ks.Set(1.0, 1.0, 1.0);

	double ns = 16.0;
	for (int i = 0; i < Depth; ++i)
	{
		for (int j = 0; j < Height; ++j)
		{
			for (int k = 0; k < Width; ++k)
			{
				int vidx = GetVoxelIdx(i, j, k);
				
				//밀도값에 따라 다른 색상을 줌
				if (40 <= pVolData[vidx] && pVolData[vidx] <= 100)
					Phong(vidx, L, Skin);
				else if (100 < pVolData[vidx] && pVolData[vidx] < 300)
					Phong(vidx, L, Bone);
				else
					Phong(vidx, L, ETC);
			}
		}
	}
}

void Phong(int vidx, GVec3 L, GSphere Obj )
{
	GLight Light;
	Light.Id.Set(1.0, 1.0, 1.0);
	Light.Is.Set(1.0, 1.0, 1.0);

	GVec3 N = pNormal[vidx];
	GVec3 R = -L + 2.0 * (N * L) * N;
	GVec3 V = L;

	GVec3 Diff = GVec3(1.0 * Obj.Kd[0], 1.0 * Obj.Kd[1],1.0 * Obj.Kd[2]);
	GVec3 Spec = GVec3(Light.Is[0] * Obj.Ks[0], Light.Is[1] * Obj.Ks[1], Light.Is[2] * Obj.Ks[2]);
	pColor[vidx] = Diff * MAX(0.0, N * L) + Spec * pow(MAX(0.0, V * R), 16.0);

}

void CreateImage()
{
	int MaxIdx = Width * Height * Depth;
	for (int i = 0; i < Depth; ++i)
	{
		for (int j = 0; j < Width; ++j)
		{
			float c = j -128;
			float s = -128;
			GPos3 p(i, 128 + c * cos(theta * (PI/180)) - s * sin(theta * (PI / 180)), 
					   128 + c * sin(theta * (PI / 180)) + s * cos(theta * (PI / 180)));
			GVec3 q(0.0, cos( (90+theta) * (PI / 180)),  sin( (90+theta) * (PI / 180)));
			q.Normalize();
			GLine ray(p, q);

			L = GVec3(0.0, -q[2], -q[1]);

			float t = 0.0;
			float alpha_out = 0.0;
			GVec3 color_out(0.0,0.0,0.0);

			while (true)
			{
				t += 0.3;
				GPos3 s = ray.Eval(t);
				float ratio[] = { ceil(s[0])-s[0], ceil(s[1])-s[1], ceil(s[2])-s[2] };

				if (t > Height) break;

				if (ceil(s[0]) > Depth-1 || ceil(s[2]) > Width-1 || ceil(s[1]) > Height-1 ||
					floor(s[0]) < 0 || floor(s[1]) < 0 || floor(s[2]) < 0 ) continue;

				if (alpha_out >= 1.0) break;

				/*
				int i = (int)s[0]; x->i
				int k = (int)s[1]; y->k
				int j = (int)s[2]; z->j

				GetVolexIndex(i,j,k);
				*/
				
				//삼선형 보간을 위한 8개의 복셀설정
				AlphaColor v1(floor(s[0]), floor(s[1]), floor(s[2]));
				AlphaColor v2(floor(s[0]), floor(s[1]), ceil(s[2]));
				AlphaColor v3(floor(s[0]), ceil(s[1]), floor(s[2]));
				AlphaColor v4(floor(s[0]), ceil(s[1]), ceil(s[2]));
				AlphaColor v5(ceil(s[0]), floor(s[1]), floor(s[2]));
				AlphaColor v6(ceil(s[0]), floor(s[1]), ceil(s[2]));
				AlphaColor v7(ceil(s[0]), ceil(s[1]), floor(s[2]));
				AlphaColor v8(ceil(s[0]), ceil(s[1]), ceil(s[2]));

				//삼선형 보간
				AlphaColor* v12 = InterpolationPos(v1, v2, ratio[2]);
				AlphaColor* v34 = InterpolationPos(v3, v4, ratio[2]);
				AlphaColor* v56 = InterpolationPos(v5, v6, ratio[2]);
				AlphaColor* v78 = InterpolationPos(v7, v8, ratio[2]);

				AlphaColor* v1234 = InterpolationPos(*v12, *v34, ratio[1]);
				AlphaColor* v5678 = InterpolationPos(*v56, *v78, ratio[1]);

				AlphaColor* in = InterpolationPos(*v1234, *v5678, ratio[0]);

				color_out = color_out + in->a * (1 - alpha_out) * in->c;
				alpha_out = alpha_out + in->a * (1 - alpha_out);

			}

			int idx = GetPixelIdx(i, j);
			color_out[0] = MIN(color_out[0], 1.0);
			color_out[1] = MIN(color_out[1], 1.0);
			color_out[2] = MIN(color_out[2], 1.0);

			pImage[idx] = color_out[0] * 255;
			pImage[idx + 1] = color_out[1] * 255;
			pImage[idx + 2] = color_out[2] * 255;

		}
	}	
}

AlphaColor* InterpolationPos(AlphaColor A, AlphaColor B, float ratio)
{
	float a = ratio * A.a + (1 - ratio) * B.a;
	GVec3 c = ratio * A.c + (1 - ratio) * B.c;
	AlphaColor ac(a,c);
	
	return &ac;
}
