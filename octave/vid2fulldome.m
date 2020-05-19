pkg load image

flat = im2double(flipud(imread('Downloads/still2.png')));
printf('Image loaded.\n');
fflush(stdout);
N=256;
flat = imresize(flat, [N,N]);
%figure; imshow(equi);
printf('Image resized.\n');
fflush(stdout);

% for remap style mapping, must do the inverse transform,
% from fisheye(X,Y) to equi(x,y)

aperture = pi;

anglexrad = 0;

% normalizing to [-1, 1]
xfishr = 2.*(1:N) / N .- 1;
yfishc = (2.*(1:N) / N .- 1)';
for i=1:N
 xfish(i,:) = xfishr;
 yfish(:,i) = yfishc;
end
rfish = sqrt(xfish.*xfish + yfish.*yfish);
theta = atan2(yfish, xfish) + anglexrad;
phi = rfish*aperture/2;

% Xindex in range [1 map_x.cols] maps to theta in [anglexrad-vidw/2, anglexrad+vidw/2]
% Yindex in range [map_x.rows 1] maps to phi in [angleyrad-halfheight, angleyrad+halfheight], ie. inverted

thetamax = 60 * pi / 180;
thetamin = 20 * pi / 180;
phimax = 90 * pi / 180;;
phimin = (90-22.5) * pi / 180;

Xindex = round((N-1) * (theta-thetamin) / (thetamax-thetamin) + 1);
Yindex = round((1-N) * (phi-phimin) / (phimax-phimin) + 1);

printf('Found indices. \n');
fflush(stdout);

%figure; surf(X,Y,  "CData", equi, "FaceColor", "texturemap");
%figure; surf(X,  "CData", equi, "FaceColor", "texturemap");
%figure; surf(Y,  "CData", equi, "FaceColor", "texturemap");

fishe = zeros(size(flat));

for i=1:N
 for j=1:N
  if(Xindex(i,j)<=N && Yindex(i,j)<=N && Xindex(i,j)>0 && Yindex(i,j)>0)
   fishe(i,j,:)=flat(Yindex(i,j),Xindex(i,j),:);
  end
 end
end

imshow(fishe)