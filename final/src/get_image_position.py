import utils
import numpy as np
from sklearn.decomposition import PCA
import pandas as pd

def get_image_position():
	#Getting pandas dataframe from csv
	image_features = utils.get_dataframe_from_csv("features.csv")

	rgb_3d_hist = image_features['img_norm_hist3d']#.reshape()
	log_histogram = image_features['norm_hist']
	pca_fft = image_features['ffts_transformed']

	#Reshaping pandas series into a more suitable format
	rgb_3d_hist = [a.reshape(1, 125) for a in rgb_3d_hist]
	rgb_3d_hist = np.array(rgb_3d_hist).reshape(999,125)

	log_histogram = [a.reshape(1,256) for a in log_histogram]
	log_histogram = np.array(log_histogram).reshape(999, 256)

	pca_fft = [a.reshape(1,20) for a in pca_fft]
	pca_fft = np.array(pca_fft).reshape(999, 20)

	#Concatenating all features and linearizing it
	all_features = np.concatenate((rgb_3d_hist, log_histogram, pca_fft), axis=1)

	#Performing PCA over three first components
	pca = PCA(n_components=3)
	pca.fit(all_features)

	#adding the column fft to data
	transformed_features = pca.transform(all_features)

	#Normalizing features
	transformed_features = transformed_features / np.max(transformed_features)

	#Getting filenames and positions into a csv file
	image_features['img_3d_position'] = pd.Series(list(transformed_features))

	positions = image_features[['filename', 'img_3d_position']].copy()

	positions.to_csv("positions_all_features.csv", index=False)

if __name__ == "__main__":
	get_image_position()


