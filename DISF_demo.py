from disf import DISF_Superpixels
from PIL import Image
import matplotlib.pyplot as plt
import numpy as np

img = np.array(Image.open("man.png"), dtype=np.int32);
num_init_seeds = 8000;
num_final_superpixels = 50;

label_img,border_img = DISF_Superpixels(img, num_init_seeds, num_final_superpixels);

fig = plt.figure();
fig.add_subplot(1,2,1);
labelplot = plt.imshow(label_img, cmap = 'gray', vmin = 0, vmax = np.max(label_img));
fig.add_subplot(1,2,2);
borderplot = plt.imshow(border_img ,cmap='gray', vmin = 0, vmax = np.max(border_img));
plt.show();


