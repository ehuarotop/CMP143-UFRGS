import multiprocessing
import subprocess
import cv2 as cv
import os
import psycopg2

def execParallel(function_name, args, data_to_be_processed):
    '''
    Desc: parallel execution of the indicated function
    parameters: 
        function_name: (str) name of the function to execute in parallel
        args: (list) arguments received by the function, except the list of elements to process.
        data_to_be_processed: (list) complete list of elements that have to be processed.
    returns:
        header: if exists
        lines: list with all of the lines of the csv.
    '''

    procs = []

    #Getting number of cpus
    number_of_cpus = multiprocessing.cpu_count()
    
    #Getting the length of every group of elements to be processed.
    length_group = len(data_to_be_processed) // number_of_cpus

    init = 0
    end = length_group
    
    #start_time = time.time()
    with multiprocessing.Manager() as manager:
        L = manager.list()  # shared between processes.
        
        for i in range(number_of_cpus):
            files = data_to_be_processed[init:end]

            if i == (number_of_cpus-1):
                files = data_to_be_processed[init:]

            #setting args (args has to be a python list)
            args_ = [files] + list(args) + [L]
            args_ = tuple(args_)

            process = multiprocessing.Process(target=function_name, args=args_)
            process.start()
            procs.append(process)

            #updating control variables
            init = end
            end += length_group

        #print("Number of procs: " + str(len(procs)))

        #Completing jobs
        for proc in procs:
            proc.join()
            
        #end_time = time.time()
        
        return list(L)

def readTxtFile(filename):
    #Reading file recently generated
    f = open(filename)
    lines = f.readlines()

    #Removing \n from each image
    lines = [x[:-1] for x in lines]

    #Sorting the list
    #lines.sort()

    return lines

def get_pdf_list(pdfs_path):
    #Getting pdf files list
    os.system("find " + pdfs_path + " -name *.pdf > ../tmp/list_of_pdfs.txt")

    #Reading list of files obtained before.
    pdfs = readTxtFile("../tmp/list_of_pdfs.txt")

    return pdfs

def get_file_list(path):
    #Getting list of extracted images in order to iterate over it
    result_find = subprocess.run(['find', path], stdout=subprocess.PIPE)
    result_find_string = result_find.stdout.decode("utf-8")
    
    #Getting information per image
    path_images = result_find_string.splitlines()

    #Removing first element(directory itself)
    return path_images[1:]

def read_image(image_path, type='normal'):
    if type == 'normal':
        img = cv.imread(image_path)
        b,g,r = cv.split(img)
        img = cv.merge([r,g,b])
    elif type == 'gray':
        img = cv.imread(image_path, 0)
    elif type == 'cmyk':
        img = Image.open(image_path)
        img = img.convert('RGB')
        img = np.asarray(img)
        
    return img

def exec_custom_sql(sql_query):
    con = psycopg2.connect(database="geocsv", user="geocsv_user", password="geocsv_database", 
                                    host="127.0.0.1", port="5432")

    cur = con.cursor()
    cur.execute(sql_query)
    rows = cur.fetchall()

    con.close()

    return rows