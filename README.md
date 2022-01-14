# RayTracing Program 
#### 프로그램 사진
<div>
<img width="250" src = "https://user-images.githubusercontent.com/45874696/67154388-9bd93e00-f336-11e9-980f-8054df29ede0.png">
<img width="250" src = "https://user-images.githubusercontent.com/45874696/67154374-3a18d400-f336-11e9-8ba8-f8bfafba780c.png">
<img width="250" src = "https://user-images.githubusercontent.com/45874696/67154377-50269480-f336-11e9-91c2-0dad29a50bc0.png">
</div>



#### 사용한 툴
  Visual Studio / C++ / OpenGL
  
#### 내용 
  C++과 OpenGL을 이용해 볼륨데이터의 시각화 프로그램 제작 
  
#### 개요

사람의 단층촬영데이터를 읽어들여 3D모델이미지로 보여주는 RayCasting 구현프로그램입니다.  
RayCasting을 적용해 3D모델의 투명도를 조절하면, 피부에서 뼈까지 단계적으로 볼 수 있도록 구현하였습니다.  
키보드로 투명도 설정, 모델의 회전이 가능합니다. 쉐이더의 조명모델은 Phong공식을 적용하였습니다.

학교 과제로 만든 프로젝트이며 단층데이터를 읽어오고 창을 띄우는 기본적 openGL코드는 교수님께 받은 뼈대 코드를 바탕으로 하였습니다.

[소스코드](RayCast.cpp)

ComputeNormal, 	AssignOpacity, ComputeColor , CreateImage 부분의 구현을 담당하였습니다.
