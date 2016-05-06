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
                root(make_shared<INodeDirectory>(string("/"), string("raftfs"), nullptr)),
                commited_index(0)
        {
            // the parent of root is itself
            root->SetParent(root.get());

        }

        FSNamespace::~FSNamespace() {
            // TODO: delete everything one by one.
        }


        bool FSNamespace::ValidatePath(const std::string &path) {
            return !path.empty() && path[0] == '/';
        }

        // not thread safe
        std::vector<std::string> FSNamespace::SplitPath(string abs_path) const {
            vector<string> dir_split;
            boost::split(dir_split, abs_path, boost::is_any_of("/"));
            dir_split.erase(dir_split.begin());
            return dir_split;
        }

        // not thread safe
        pair<INodeDirectory *, string> FSNamespace::GetParentFolderAndChild(const string &abs_path) const {

            if (abs_path == "/" ) {
                return make_pair(root.get(), string());
            }

            auto dir_split = SplitPath(abs_path);
            auto current = root.get();

            // lock_guard<mutex> guard(m);
            for (int i = 0; i < dir_split.size() - 1; ++i) {
                auto next = current->GetChild(dir_split[i]);
                if (next && next->IsDir()) {
                    current = static_cast<INodeDirectory *>(next);
                } else {
                    return make_pair(nullptr, string());
                }
            }
            return make_pair(current, dir_split.back());
        }

        bool FSNamespace::MakeDir(const string abs_dir, const std::string &owner, bool make_parents) {
            INodeDirectory* current = root.get();

            // todo find other way to validate path
            //assert(abs_dir[0] == '/');

            auto dir_split = SplitPath(abs_dir);

            int new_dir = 0, i;

            std::lock_guard<std::mutex> guard(m);

            for (i = 0; i < dir_split.size(); ++i) {
                // dir exist
                //cout << "sub path " << dir_split[i] << endl;
                auto sub = current->GetChild(dir_split[i]);
                if (sub) {
                    current = static_cast<INodeDirectory *>(sub);
                } else {
                    // missing middle level
                    //cout << "missing " << dir_split[i] << endl;
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

            lock_guard<mutex> lock(m);

            auto parent_child = GetParentFolderAndChild(abs_dir);

            if (parent_child.first && !parent_child.second.empty()) {
                auto target = parent_child.first->GetChild(parent_child.second);
                if (target && target->IsDir() && (target->GetOwner() == visitor || target->GetOwner() == "unknown"))
                    return parent_child.first->DeleteChild(parent_child.second, recursive);
            }

            return false;
        }

        bool FSNamespace::CreateFile(const std::string &new_file, const std::string &owner) {
            std::lock_guard<std::mutex> guard(m);

            auto parent_child = GetParentFolderAndChild(new_file);

            if (parent_child.first && !parent_child.second.empty()) {
                return parent_child.first->CreateFile(parent_child.second) != nullptr;
            }
            return false;
        }

        bool FSNamespace::RemoveFile(const std::string &file_name, const std::string &visitor) {

            lock_guard<mutex> lock(m);

            auto parent_child = GetParentFolderAndChild(file_name);

            if (parent_child.first && !parent_child.second.empty()) {
                auto target = parent_child.first->GetChild(parent_child.second);
                if (target && target->IsFile() && (target->GetOwner() == visitor || target->GetOwner() == "unknown"))
                    return parent_child.first->DeleteChild(parent_child.second, false);
            }
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

            lock_guard<mutex> lock(m);

            auto parent_child = GetParentFolderAndChild(abs_dir);

            if (parent_child.first) {
                if (parent_child.second.empty())
                    return parent_child.first->ListDirName();

                auto target_dir = static_cast<INodeDirectory *>(parent_child.first->GetChild(parent_child.second));

                if (target_dir) {
                    return target_dir->ListDirName();
                }
            }
            return std::vector<std::string>();
        }

        bool FSNamespace::GetFileInfo(const std::string &filename, protocol::FileInfo &info) const {

            lock_guard<mutex> guard(m);

            auto parent_child = GetParentFolderAndChild(filename);

            if (parent_child.first && !parent_child.second.empty()) {
                auto target_file = static_cast<INodeFile *>(parent_child.first->GetChild(parent_child.second));
                if (!target_file) {
                    return false;
                }
                target_file->ToFileInfo(info);
            }
            return true;
        }


        void FSNamespace::Print() const {
            root->Print(1);
        }

        std::ostream &operator<<(std::ostream &os, const FSNamespace &fs) {
            auto curr = fs.root;

            return os;
        }


        std::string OpToStr(const MetaOp::type op) {
            switch (op) {
                case MetaOp::kCreate:
                    return string("CreateFile");
                case MetaOp::kDelete:
                    return string("DeleteFile");
                case MetaOp::kMkdir:
                    return string("MKDir");
                case MetaOp::kRmdir:
                    return string("RMDir");
                case MetaOp::kGetFileInfo:
                    return string("FileInfo");
                case MetaOp::kListDir:
                    return string("ListDir");
                default:
                    return string("Noop");
            }
        }


    } // namespace raftfs::filesystem
} // namespace raftfs
