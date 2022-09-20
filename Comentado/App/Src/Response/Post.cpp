#include "Response.hpp"

/**
 * If the request is a multipart/form-data request, then call responseMultipart(), otherwise call
 * handleCgi()
 */
void Response::responsePost(){
	if(request->getContentType().find("multipart/form-data") != npos)
		responseMultipart();
	else
		handleCgi();
};

/**
 * It handles the multipart/form-data request
 * 
 * @return the number of bytes that were written to the file.
 */

void Response::responseMultipart(){
	boost::string boundary("");
	size_t start = 0, pos = 0;

	int i = findHost();
	if (i != -1){
		for (size_t j = 0; j < server->sockVec.at(i).getServInfo().locations.size(); j++){
			if (request->getUrl() == server->sockVec.at(i).getServInfo().locations.at(j).name){
				upload_dir = server->sockVec.at(i).getServInfo().locations.at(j).upload_dir;
				break;
			}
		}
	} else {
		status_code = STATUS_SERVER_ERROR;
		errorBody();
		makeHeader();
		return ;
	}

	boundary = request->getContentType();
	size_t n = boundary.find("--");
	boundary.erase(0, n);
	boundary.insert(0, "--");
	boundary.append("--");

	if (server->getBinBoundary() == "")
		server->setBinBoundary(boundary);
	if (server->getIsChunked() == false){
		if (conf.client_max_body_size < request->getContentLength()){
			status_code = STATUS_TOO_LARGE;
			makeHeader();
			server->setTooLarge(true);
			return ;
		}
		setFileName();
		start = findBodyStart();
		if (!folderExist(path)){
			handleFile();
		}
		if (server->getBytes() == 4096)
			server->setIsChunked(true);
	}
	pos = findBodyEnd(start, server->getBinBoundary());
	writeToFile(start, pos);
	if (server->getBytes() != 4096){
		handleAutoindex("./www/upload");
		makeHeader();
		server->setIsChunked(false);
	}
};

/**
 * It writes the data from the buffer to the file
 * 
 * @param start the start of the file in the buffer
 * @param end the end of the file
 */
void Response::writeToFile(size_t start , size_t end){
	std::ofstream ofs(server->uploadPath().c_str() , std::ofstream::out | std::ofstream::app | std::ofstream::binary);
	if (!ofs.good() || !ofs.is_open())
		std::cout << "Error in file path out" << std::endl;	
	const char * addr = &request->buffer[start];
	size_t len = 0;
	if (server->getBytes() != 4096)
		len = end - 5 - start;
	else 
		len = end - start;
	ofs.write(addr, len);
	ofs.close();
	if (!ofs.good())
		std::cout << "Error writing in file" << std::endl;
};

/**
 * It finds the start of the body of the request
 * 
 * @return The start of the body of the request.
 */
size_t Response::findBodyStart(){
	size_t start = 0;
	std::stringstream hold;
	hold << request->buffer;
	start = hold.str().find("filename=\"", 0);
	start = hold.str().find("\r\n\r\n", start);
	start += 4;
	return start;
};

/**
 * It finds the end of the body
 * 
 * @param start The starting point of the search
 * @param boundary The boundary string that separates the body from the headers.
 * 
 * @return The size of the buffer.
 */
size_t Response::findBodyEnd(size_t start, boost::string boundary){
	char *ptr;
	size_t max_length = 0;
	while (start < sizeof(request->buffer)){
		max_length = sizeof(request->buffer) - start;
		ptr = (char*)memchr(request->buffer + start, '-', max_length);
		if (!ptr || start > sizeof(request->buffer)){
			start = server->getBytes();
			return start;
		}
		start = ptr - request->buffer + 1;
		if (memcmp(ptr, boundary.c_str(), boundary.size()) == 0)
			break;
	}
	return start + 1;
};

/**
 * It parses the request buffer and extracts the filename of the uploaded file
 */
void Response::setFileName(){
	size_t start = 0, pos = 0;
	std::stringstream hold;
	hold << request->buffer;
	pos = hold.str().find("filename=\"", 0);
	if (pos != npos){
		pos += 10;
		start = hold.str().find('"', pos);
		if (start != npos){
			for (size_t i = 0; i < conf.locations.size(); i++){
				if (conf.locations.at(i).upload_dir != ""){
					path.assign(conf.locations.at(i).upload_dir);
					break;
				}
			}
			if (!folderExist(path))
				mkdir(path.c_str(), ACCESSPERMS);
			path.append(hold.str().substr(pos, start - pos));
			server->setUploadPath(path);
		}
	}
};

/**
 * It deletes the file that was uploaded
 */
void Response::handleFile(){
	std::ifstream infile(server->uploadPath().c_str());
	if (infile.good()){
		std::cout << "O arquivo já exite" << std::endl;
		if (remove(server->uploadPath().c_str()) == -1)
			std::cout << "nao foi possivel apagar o arquivo" << std::endl;
		else
			std::cout << "arquivo apagado com sucesso" << std::endl;
	}
};
