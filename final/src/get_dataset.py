import psycopg2
import os

petr_dataset = "/home/ehuarotop/PetrDataset"

classes = ["UFRGS:GeoImage_Photomicrograph", "UFRGS:GeoImage_outcrop_photograph", "UFRGS:GeoImage_map",
			"UFRGS:GeoImage_profile", "UFRGS:GeoImage_3d_block_diagram", "UFRGS:GeoImage_seismic_section",
			"UFRGS:GeoImage_hand_sample_photograph", "UFRGS:GeoImage_cross_section",
			"UFRGS:GeoImage_stratigraphic_profile", "UFRGS:GeoImage_satellite_image"]

def exec_custom_sql(sql_query):
    con = psycopg2.connect(database="geocsv", user="geocsv_user", password="geocsv_database", 
                                    host="127.0.0.1", port="5432")

    cur = con.cursor()
    cur.execute(sql_query)
    rows = cur.fetchall()

    con.close()

    return rows

def main():
	for classe in classes:
		imagehash_list = exec_custom_sql("""select image_id from images_labeledimage il
								where il.label_id = '{0}' limit 100""".format(classe))

		english_name = exec_custom_sql("""select label_english_name 
										from images_label il where label_id = '{0}'""".format(classe))

		english_name = english_name[0][0]

		#Creating directory for the current class
		os.system("""mkdir -p 'dataset/{0}'""".format(english_name))

		for image in imagehash_list:
			image_path = exec_custom_sql("""select ii.repo_dir || ii.image_name 
											from images_imagepath ii where image_id = '{0}' 
											limit 1""".format(image[0]))

			image_path = image_path[0][0]

			print(image_path)

			os.system("""cp '{0}' '{1}'""".format(os.path.join(petr_dataset, image_path), 
										"dataset/{0}".format(english_name)))

if __name__ == "__main__":
	main()