/*
 * VirtualInode.cpp
 *
 *  Created on: Apr 22, 2016
 *      Author: huang630
 */

#include "INode.h"
#include "../utils/time_utils.h"
#include <iostream>
#include <mutex>
//#include <algorithm>
#include <vector>
#include <string.h>
#include <cstddef>
#include <sstream>
#include <chrono>


using namespace std;
using namespace std::chrono;
using namespace raftfs::protocol;

namespace raftfs {
    namespace filesystem {

    	/*====================================================
    	 *  INode: Upper level abstract class
    	 *  - Different behavior regarding file and dir should be defined
    	 *    in lower layer INodeFile and INodeDirectory
    	 *====================================================*/
        INode::INode(const string _name, const string _owner, INode *_parent)
            : name(_name),
              owner(_owner),
              parent(_parent),
              create_time(Now()),
              modification_time(create_time),
              access_time(create_time)
        {

        }


        INode::INode(const INode &inode)
            : name(inode.name),
              owner(inode.owner),
              // two inodes with the same name cannot exist in the same folder
              // parent need to set manully later
              parent(nullptr),
              create_time(inode.create_time),
              modification_time(inode.modification_time),
              access_time(inode.access_time)
        {

        }

        INode &INode::operator=(const INode &inode) {
            if (this != &inode) {
                name = inode.name;
                owner = inode.owner;
                parent = nullptr;
                create_time = inode.create_time;
                modification_time = inode.modification_time;
                access_time = inode.access_time;
            }
            return *this;
        }

        INode::~INode() {

        }

        // fixme: maybe do not need this function
        // create another function like ValidPathName(path) in FSNamespace class
        bool INode::ValidName(const std::string &name) {
			#if(0)
        	if (!name.empty() && name[0] == '/')
				return true;
			#endif

        	if (name[0] != '/')	return false;	// must contain '/'?

			// The name should not contains special characters.
			string error_list = ",><[]{}|~&^*()";
			bool error = false;
			for(int i=0; i<error_list.size(); ++i) {
				if(name.find( error_list[i] ) != string::npos) {
					error = true;
					break;
				}
			}
            return !error;
		}


    	/*====================================================
    	 *
    	 *  INodeFile: Final level class
    	 *
    	 *====================================================*/

        INodeFile::INodeFile(const string _name, const string _owner, INode *_parent)
            : INode(_name, _owner, _parent),
              size(0)
        {

        }


        INodeFile::INodeFile(const string _name, INode *_parent)
            :INode(_name, string("unknown"), _parent)
        {

        }


        INodeFile::INodeFile(const INodeFile &iNodeFile)
            : INode(iNodeFile),
              size(iNodeFile.size),
              data(iNodeFile.data)
        {

        }

        INodeFile &INodeFile::operator=(const INodeFile &iNodeFile) {
            if (this != &iNodeFile) {
                INode::operator=(iNodeFile);
                size = iNodeFile.size;
                data = iNodeFile.data;
            }
            return *this;
        }

        void INodeFile::ToFileInfo(protocol::FileInfo &info) const {

            info.path = name;
            auto p = parent;
            while (p) {
                info.path = p->GetName() + "/" + info.path;
                p = p->GetParent();
            }
            info.create_time = duration_cast<seconds>(create_time.time_since_epoch()).count();
            info.creator = owner;
            info.size = size;
        }


    	/*====================================================
    	 *
    	 *  INodeDirectory: Final level class
    	 *
    	 *====================================================*/


        INodeDirectory::INodeDirectory(const std::string _name, const std::string _owner, INode *_parent)
            : INode(_name, _owner, _parent)
        {

        }

        INodeDirectory::INodeDirectory(const std::string _name, INode *_parent)
            : INode(_name, string("unknown"), _parent)
        {

        }


        INode * INodeDirectory::GetChild(const std::string& child_name) const {
			lock_guard<mutex> guard(m);
			auto it = children_map.find(child_name);
            if (it == children_map.end())
			    return nullptr;
            else
                return it->second;
        }



        bool INodeDirectory::CreateFile(const string &file_name) {
        	// don't allow two INodes have same name
        	if(GetChild(file_name)) {
                return false;
            }

            INode* file = new INodeFile(file_name, this);
            AddChild(file);
        	return true;
        }


        bool INodeDirectory::AddChild(INode *new_child) {

            lock_guard<mutex> guard(m);
            shared_ptr<INode> ptr;
            ptr.reset(new_child);
            if (!ptr->GetParent() || ptr->GetParent() != this)
                ptr->SetParent(this);
            children.push_back(ptr);
            children_map[ptr->GetName()] = children.back().get();

            return true;
        }


        std::vector<std::string> INodeDirectory::ListDirName() const {
            // TODO
            return std::vector<std::string>();
        }

        std::vector<INode *> INodeDirectory::ListDirINode() const {
            // todo
            return std::vector<INode *>();
        }

        bool INodeDirectory::CreateDir(std::string &dir_name) {
        	// don't allow two INodes have same name
        	if(GetChild(dir_name))
                return false;

            INodeDirectory * dir = new INodeDirectory(dir_name, this);
            AddChild(dir);

            return true;
        }

        bool INodeDirectory::DeleteChild(const std::string &child_name, bool recursive) {
            // todo
        	INode* target = GetChild(child_name);
        	if(!target)	return false;

    		if (target->IsFile()) {	// ignore recursive if IsFile
    			//delete target;
    			children_map.erase(child_name);
    			return true;
    		} else {
    			// IsDir
            	if(recursive) {
            		// TODO: check if crash


            	} else {
            		if( ((INodeDirectory*)target)->IsEmpty() ) {
            			delete target;
            			children_map.erase(child_name);
            		}else {
            			return false;
            		}
            	}
    		}
            return false;
        }

        bool INodeDirectory::DeleteAllChild() {
        	for(auto it: children) {
        		if(it->IsFile()) {
					it.reset();
				} else {	// IsDir()
					((INodeDirectory*)children_map[it->GetName()])->DeleteAllChild();
					it.reset();
				}
        		children_map.erase(it->GetName());
        	}
            return true;
        }

        std::ostream& operator<<(std::ostream& os, const INode& node) {
        	os << node.GetName();
            return os;
        }


        std::ostream& operator<<(std::ostream &os, const INodeFile &node) {
            os << node.name ;

            return os;
        }

        std::ostream& operator<<(std::ostream &os, const INodeDirectory &node) {
            os << node.name << endl;
            for (auto& n : node.children) {
                if (n->IsFile())
                    cout << "  ->" << *static_pointer_cast<INodeFile>(n) << endl;
                else if (n->IsDir())
                    cout << " |->" << *static_pointer_cast<INodeDirectory>(n) << endl;
                else {
                    cout << " WRONG " << *n << "is not file and not dir" << endl;
                }
            }
            return os;
        }


    } // namespace raftfs::server
} // namespace raftfs
