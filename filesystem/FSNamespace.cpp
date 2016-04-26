/*
 * VirtualInode.cpp
 *
 *  Created on: Apr 22, 2016
 *      Author: huang630
 */

#include "FSNamespace.h"
#include "INode.h"
#include <iostream>
#include <mutex>
#include <algorithm>
#include <cstddef>
#include <sstream>
#include <cassert>

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
                root(make_shared<INodeDirectory>(string(), string("raftfs"), nullptr)) {
            // the parent of root is itself
            root->SetParent(root.get());

        }

        FSNamespace::~FSNamespace() {
            // TODO: delete everything one by one.
        }

        /*
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
        */
        bool FSNamespace::MakeDir(const string abs_dir, const std::string &owner, bool make_parents) {
            std::lock_guard<std::mutex> guard(m);
            INodeDirectory* current = root.get();

            // todo find other way to validate path
            assert(abs_dir[0] == '/');

            istringstream f(abs_dir);
            string next_lvl;
            //
            while (!f.eof() && getline(f, next_lvl, '/')) {

                if (next_lvl.empty())
                    continue;

                INode* nextdir = current->GetChild(next_lvl);

                // TODO: now we make parents anyway... ignoring "make_parents"
                if (!nextdir) {
                    //-- Reach final level -> Create if not exist
                    if (current->CreateDir(next_lvl)) {
                        nextdir = current->GetChild(next_lvl);
                    } else {
                        return false;
                    }
                } else {
                    //-- Advance to next level
                    cout << "Next level: " << next_lvl << endl;
                    if (nextdir->IsDir()) {
                        current = static_cast<INodeDirectory *>(nextdir);
                    }else{
                    	return false;	// existing a file in the middle, not dir.
                    }
                }
            }
            return true;
        }


        bool FSNamespace::DeleteDir(const std::string &abs_dir, const std::string &visitor, bool recursive) {
            return false;
        }

        bool FSNamespace::CreateFile(const std::string &new_file, const std::string &owner) {
            std::lock_guard<std::mutex> guard(m);
            INodeDirectory* current = root.get();

            // todo find other way to validate path
            assert(new_file[0] == '/');

            istringstream f(new_file);
            string next_lvl;
            //
            while (!f.eof() && getline(f, next_lvl, '/')) {

                if (next_lvl.empty())
                    continue;

                INode* nextdir = current->GetChild(next_lvl);

                if (!nextdir) {
                	if(f.eof()) {
                        //-- Reach final level -> Create if not exist
                        if (current->CreateFile(next_lvl)) {
                        	return true;
                        } else {
                        	return false;	// existing file.
                        }
                    } else {
                        return false;	// no middle dir.
                    }
                } else {
                    //-- Advance to next level
                    cout << "Next level: " << next_lvl << endl;
                    if (nextdir->IsDir()) {
                        current = static_cast<INodeDirectory *>(nextdir);
                    }else{
                    	return false;	// existing a file in the middle, not dir.
                    }
                }
            }
            return false;
        }

        bool FSNamespace::RemoveFile(const std::string &file_name, const std::string &visitor) {
            return false;
        }

        bool FSNamespace::UpdateFile(const std::string &file_name, const std::string &visitor,
                                     protocol::FileInfo new_info) {
            return false;
        }

        bool FSNamespace::ExistInode(const std::string &abs_path) const {
            return false;
        }

        std::vector<std::string> FSNamespace::ListDir(const std::string &abs_dir) const {
            return std::vector<std::string>();
        }

        protocol::FileInfo FSNamespace::GetFileInfo(const std::string &filename) const {
            return protocol::FileInfo();
        }


        /*
         * Output formating functions
         */
        void FSNamespace::output_space(int a) {
            char spaces[256];
            memset(spaces, ' ', 255);
            spaces[255] = '\0';
            spaces[a * 2] = '\0';
            cout << spaces;
        }


        std::ostream &operator<<(std::ostream &os, const FSNamespace &fs) {

        }

    } // namespace raftfs::filesystem
} // namespace raftfs
