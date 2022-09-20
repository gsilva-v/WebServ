#include "Response.hpp"
/**
 * The function deletes the file at the path specified in the request
 */
void Response::responseDelete(){
	deletePath(path);
	status_code = STATUS_OK;
	makeHeader();
};

/**
 * It deletes a directory and all its contents
 * 
 * @param path the path of the folder to be deleted
 */
void Response::deletePath(boost::string path){
	DIR *dir = opendir(path.c_str());
	dirent * dire;
	int ret = 0;
	boost::string line;

	if (!dir){
		if (fileExist(path)){
			unlink(path.c_str());
			return ;
		}
		status_code = STATUS_SERVER_ERROR;
		errorBody();
		std::cout << "delete error" << std::endl;
		return ;
	}
	while ((dire = readdir(dir)) != NULL) {
		line.clear();
		line = path;
		if (!line.ends_with('/'))
			line.append("/");
		line.append(dire->d_name);
		if (line.ends_with('.'))
			continue;
		if (dire->d_type == DT_DIR)
			line.append("/");
		
		if (folderExist(line)){
			deletePath(line);
			ret = rmdir(line.c_str());
		} else 
			ret = unlink(line.c_str());
		if (ret < 0)
			std::cout << "Delete error" << std::endl;
	}
	closedir(dir);	
};
