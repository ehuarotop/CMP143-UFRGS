#import built-in libraries
import sys
import os
import csv

#import for custom libraries
import cv2 as cv
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import curve_fit
import pandas as pd


#Importing PCA
from sklearn.decomposition import PCA

#Importing utils
import utils

#settings for get the correct arrays
np.set_printoptions(threshold=sys.maxsize)
csv.field_size_limit(sys.maxsize)

#List to save all information for every images
images = []

#Defining main functions to be used
def get_histogram(file, abs_value=False, downsample=1):
    print(file)
    #reading the image into an mxnx3 array
    img = cv.imread(file)

    #getting pixel values in float32
    img = img.astype(np.float32)

    #splitting color channels
    b,g,r = cv.split(img)
    #merging them in RGB order.
    img_orig = cv.merge((r,g,b))
    
    #Calculating the 3D histogram of cores:
    rows,cols,channels = img_orig.shape
    img_vector = np.reshape(img_orig, (rows*cols,channels))
    img_hist3d = np.histogramdd(img_vector, bins=5)
    img_norm_hist3d = img_hist3d[0] / np.max(img_hist3d[0])
    i,j,k = img_norm_hist3d.shape
    img_norm_hist3d_feature_vector = np.array(np.reshape(img_norm_hist3d,  i*j*k))

    #defining the filter operator
    gfilter = np.matrix(('1.0 -1.0'))

    #applying filter to each matrix separately (image derivative)
    b = cv.filter2D(b, -1, gfilter)
    g = cv.filter2D(g, -1, gfilter)
    r = cv.filter2D(r, -1, gfilter)
    
    #If abs_value is specified
    if abs_value == True:
        b = np.abs(b)
        g = np.abs(g)
        r = np.abs(r)

    #merging color channels after filter application
    img = cv.merge((r,g,b))
    
    if abs_value == True:
        N = 256
    else:
        N = 512-1
        
    N = N // downsample

    histr = np.zeros(shape=(N,1), dtype=np.float32)

    for channel in range(3):
        #calculating the histogram
        if abs_value == True:
            histr += cv.calcHist([img],[channel],None,[N],[0,255])
        else:
            histr += cv.calcHist([img],[channel],None,[N],[-255,255])

    #normalized histogram (# norm_histr = histr / histr[0])
    norm_histr = histr/np.max(histr)
    
    return img_norm_hist3d_feature_vector, norm_histr

def adding_pca_FT_feature_to_data(data):
    #Getting all FFT images
    ffts = np.abs(data['FFTReal'] + data['FFTImag']*1j)
    ffts = [a.reshape(1,10000) for a in ffts]
    ffts = np.array(ffts).reshape(data.shape[0],10000)

    #Getting PCA components of the fft of images
    pca = PCA(n_components=20)
    pca.fit(ffts)
    
    #adding the column fft to data
    ffts_transformed = pca.transform(ffts)
    
    data['ffts_transformed'] = pd.Series(list(ffts_transformed))
    
    return data

################### Feature Generation ###################

#Getting file list from dataset directory
filenames = utils.get_file_list("dataset")

for filename in filenames:

	image = []

	if filename.endswith(".png") or filename.endswith(".jpg"):
		#getting root directory and filename for each image and append to features.
		image.append(filename)

		#getting original image and his histogram
		img_norm_hist3d_feature_vector, norm_histr = get_histogram(filename, True)

		#append histr to image list
		image.append(np.array_str(img_norm_hist3d_feature_vector))
		image.append(np.array_str(norm_histr.reshape(1,-1)))

		x_medio = 100
		y_medio = 100
		img = utils.read_image(filename, 'gray')
		img = cv.resize(img, (x_medio, y_medio))
		fft = np.fft.fft2(img)

		image.append(fft.real.reshape(1,-1))
		image.append(fft.imag.reshape(1,-1))

		images.append(image)


#Converting images list into a dataframe in order to process each dir or subdir separately.
labels = ['filename', 'img_norm_hist3d', 'norm_hist', 'FFTReal','FFTImag']
pd_images = pd.DataFrame.from_records(images, columns=labels)

#adding the other labels with blank values
pd_images = pd_images.reindex(columns = labels)

print('before save to csv')

#Saving dataframe to csv
pd_images.to_csv("features.csv", index=False)

print("Done")

################### Until here execute separately ###################

################### Getting PCA components and generating new CSV file ###################

headers = []
image_features = []

with open("features.csv", 'r') as f:
	reader = csv.reader(f)
	read_images = list(reader)

	headers = read_images[0]
	image_features = read_images[1:]

for idx, image in enumerate(image_features):
	image[1] = np.fromstring(image[1][1:-1], dtype=np.float32, sep=' ')
	image[2] = np.log(np.fromstring(image[2][2:-2],dtype=np.float32, sep=' ').reshape(256,1)[:,0] + 10**(-6))
	image[3] = np.fromstring(image[3][2:-2],dtype=np.float32, sep=' ').reshape(100,100)
	image[4] = np.fromstring(image[4][2:-2],dtype=np.float32, sep=' ').reshape(100,100)

#Getting features in pandas dataframe
image_features = pd.DataFrame.from_records(image_features, columns=headers)

#Addding PCA feature
image_features = adding_pca_FT_feature_to_data(image_features)

#Leaving only relevant features
image_features = image_features[['filename', 'img_norm_hist3d', 'norm_hist', 'ffts_transformed']].copy()

image_features.to_csv("features_with_pca.csv", index=False)


