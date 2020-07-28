import os
import subprocess

def get_file_list(path):
    #Getting list of extracted images in order to iterate over it
    result_find = subprocess.run(['find', path], stdout=subprocess.PIPE)
    result_find_string = result_find.stdout.decode("utf-8")
    
    #Getting information per image
    path_images = result_find_string.splitlines()

    #Removing first element(directory itself)
    return path_images[1:]

if __name__ == "__main__":
	files = get_file_list("../dataset/Images")

	os.system("mkdir -p '../dataset/Images2/3d block diagram'")
	os.system("mkdir -p '../dataset/Images2/cross section'")
	os.system("mkdir -p '../dataset/Images2/hand sample photograph'")
	os.system("mkdir -p '../dataset/Images2/map'")
	os.system("mkdir -p '../dataset/Images2/outcrop photograph'")
	os.system("mkdir -p '../dataset/Images2/photomicrograph'")
	os.system("mkdir -p '../dataset/Images2/profile'")
	os.system("mkdir -p '../dataset/Images2/sattelite image'")
	os.system("mkdir -p '../dataset/Images2/seismic section'")
	os.system("mkdir -p '../dataset/Images2/stratigraphic profile'")

	for file in files:
		basename = os.path.splitext(file)[0]
		extension = os.path.splitext(file)[1]

		if extension == ".png":
			print(basename.replace("dataset/Images", "dataset/Images2") + ".jpg")
			os.system("""convert '{0}' -resize 128x -quality 80 '{1}'""".format(file, basename.replace("dataset/Images", "dataset/Images2") + ".jpg"))
		elif extension == ".jpg":
			os.system("""convert '{0}' -resize 128x -quality 80 '{1}'""".format(file, file.replace("dataset/Images", "dataset/Images2")))