pkg load image; % <-- Comment this for MATLAB
addpath('./mex/');

img = imread("man.png");
img = int32(img);

[label_img, border_img] = DISF_Superpixels(img, 8000, 50);

subplot(1,2,1),imshow(uint8(label_img)),...
subplot(1,2,2),imshow(logical(border_img));
