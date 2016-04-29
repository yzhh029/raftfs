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

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;
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
                root(make_shared<INodeDirectory>(string("/"), string("raftfs"), nullptr)) {
            // the parent of root is itself
            root->SetParent(root.get());

        }

        FSNamespace::~FSNamespace() {
            // TODO: delete everything one by one.
        }


        bool FSNamespace::ValidatePath(const std::string &path) {
            return !path.empty() && path[0] == '/';
        }


        std::vector<std::string> FSNamespace::SplitPath(string abs_path) {
            vector<string> dir_split;
            boost::split(dir_split, abs_path, boost::is_any_of("/"));
            dir_split.erase(dir_split.begin());
            return dir_split;
        }


        bool FSNamespace::MakeDir(const string abs_dir, const std::string &owner, bool make_parents) {
            std::lock_guard<std::mutex> guard(m);
            INodeDirectory* current = root.get();

            // todo find other way to validate path
            assert(abs_dir[0] == '/');

            auto dir_split = SplitPath(abs_dir);

            int new_dir = 0, i;

            for (i = 0; i < dir_split.size(); ++i) {
                // dir exist
                cout << "sub path " << dir_split[i] << endl;
                auto sub = current->GetChild(dir_split[i]);
                if (sub) {
                    current = static_cast<INodeDirectory *>(sub);
                } else {
                    // missing middle level
                    cout << "missing " << dir_split[i] << endl;
                    if (i!= dir_split.size() - 1) {
                        // remember the index of the first missing level
                        if (!new_dir)
                            new_dir = i;

                        if (make_parents) {
                            // if create middle dir is allowed
                            sub = current->CreateDir(dir_split[i]);
                            // create success
                            if (sub) {
                                current = static_cast<INodeDirectory *>(sub);
                            } else {
                                break;
                            }
                        } else {
                            return false;
                        }
                    } else {
                        // reach final level, create dir
                        if (current->CreateDir(dir_split[i]) ){
                            new_dir = 0;
                        }
                    }
                }
            }

            if (new_dir && new_dir != i) {
                for (; i != new_dir - 1; --i) {
                    current = static_cast<INodeDirectory *>(current->GetParent());
                }
                current->DeleteChild(dir_split[new_dir], true);
                return false;
            }
            return true;
        }


        bool FSNamespace::DeleteDir(const std::string &abs_dir, const std::string &visitor, bool recursive) {

            auto current = root.get();
            auto dir_split = SplitPath(abs_dir);

            for (int i = 0 ; i < dir_split.size() - 1; ++i) {
                auto sub_ptr = current->GetChild(dir_split[i]);
                if (sub_ptr) {
                    current = static_cast<INodeDirectory *>(sub_ptr);
                } else {
                    break;
                }
            }
            // found the parent of last dir
            if (current->GetName() == dir_split[dir_split.size()-2] &&
                    current->GetChild(dir_split[dir_split.size()-1])->GetOwner() == visitor) {
                return current->DeleteChild(dir_split[dir_split.size()-2], recursive);
            }

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
                        return current->CreateFile(next_lvl);
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


        void FSNamespace::Print() const {
            root->Print(1);
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
            auto curr = fs.root;

            return os;
        }

    } // namespace raftfs::filesystem
} // namespace raftfs
