/*
 * VirtualInode.cpp
 *
 *  Created on: Apr 22, 2016
 *      Author: huang630
 */

#include "FSNamespace.h"
#include <iostream>
#include <mutex>
#include <algorithm>
#include <cstddef>
#include <sstream>

using namespace std;
using namespace raftfs::protocol;

namespace raftfs {
    namespace filesystem {    

    /******************************************************************
     *
     * Currently expect this is used one instance per server.
     * So we use the single lock for convenience and safety.
     *
     *******************************************************************/

    FSNamespace::FSNamespace() :
		root("/", VINODE_DIR)	{


    }

    FSNamespace::~FSNamespace() {
    	// TODO: delete everything one by one.
    }

    bool FSNamespace::Chdir(char * path) {
    	std::lock_guard<std::mutex> guard(m);
        INode * dir;
    	if(path[0] == '/') {	// trace from root
    		dir = &root;
    		if(path[1] == '\0') {	// "cd /"
    			this->curdir = &root;
    			return true;
    		}
    	} else {
    		dir = this->curdir;
    	}
    	//
        istringstream f(path);
        string next_lvl;
        bool error = false;
        while (getline(f, next_lvl, '/')) {
        	if(next_lvl.compare("") == 0)
        		continue;	// to prevent double layer "//"

        	INode * nextdir = dir->ExistChildDir(next_lvl.c_str());
        	cout << "Next level: " << next_lvl << " " << nextdir << endl;
        	if(nextdir != nullptr) {
        		dir = nextdir;
        		//if(f.eof())	break;
        	}else{
        		error = true;
        		break;
        	}
        }

        if(error)	return false;
        this->curdir = dir;
        return true;
    }

    bool FSNamespace::MakeDir(const std::string &abs_dir, const std::string &owner) {
    	std::lock_guard<std::mutex> guard(m);
        INode * dir;
    	if(abs_dir[0] == '/') {	// trace from root
    		dir = &root;
    	} else {
    		dir = this->curdir;
    	}
    	//
        istringstream f(abs_dir);
        string next_lvl;
        //
        while (getline(f, next_lvl, '/')) {
        	if(next_lvl.compare("") == 0)
        		continue;	// to prevent double layer "//"

        	INode * nextdir = dir->ExistChildDir(next_lvl.c_str());
        	//
        	if(f.eof())	{
        		//-- Reach final level -> Create if not exist
            	if(nextdir == nullptr) {
            		dir->CreateDir(next_lvl.c_str());
            		return true;
            	}else{
            		return false;
            	}
        	} else {
        		//-- Advance to next level
        		cout << "Next level: " << next_lvl << endl;
            	if(nextdir != nullptr) {
            		dir = nextdir;
            	}else{
            		return false;
            	}
        	}
        }
        return false;
    }


    const char * FSNamespace::GetCurDir() const	{
    	// FIXME
    	if(curdir == &root) {
    		return root.GetName();
    	}else{
    		return this->curdir->GetName();
    	}
    }

    /*
     * Output formating functions
     */
    void FSNamespace::output_space(int a) {
    	char spaces[256];
    	memset(spaces, ' ', 255);
    	spaces[255] = '\0';
    	spaces[a*2] = '\0';
    	cout << spaces;
    }

    void FSNamespace::list_next_lvl(INode * pnode, int lvl) {
    	output_space(lvl);
    	cout << pnode->GetName() << endl;
    	for(auto p: *(pnode->GetAllNodes())) {
    		list_next_lvl(p, lvl+1);
    	}
    }

    void FSNamespace::list() {
    	cout << root.GetName() << endl;
    	for(auto p: *(root.GetAllNodes())) {
    		list_next_lvl(p, 1);
    	}
    }
    std::ostream& operator<<(std::ostream& os, const FSNamespace& fs) {
            	os << fs.GetCurDir();
    }

#if(0)
    FSNamespace::
    bool MakeDir(char * path);
    bool MakeFile(char * path);
    bool ExistDir(char * path);
    bool ExistFile(char * path);
    bool Remove(char * path);
    const char * GetCurDir() const;

    bool Move(char * old_path, char * new_path);
#endif




    } // namespace raftfs::server
} // namespace raftfs
