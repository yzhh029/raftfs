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


using namespace std;

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


        INode::~INode() {

        }


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
            :INode(_name, string(), _parent)
        {

        }


        protocol::FileInfo INodeFile::ToFileInfo() const {
            return protocol::FileInfo();
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
        	if(!GetChild(file_name))
                return false;

        	lock_guard<mutex> guard(m);
        	children.push_back(make_shared<INodeFile>(file_name, this));
            children_map[file_name] = children[children.size() - 1].get();

        	return true;
        }


        bool INodeDirectory::AddFile(INodeFile &file) {
            if (!GetChild(file.GetName()))
                return false;

            // TODO not finished, need copy constructor -- Need verificaation
        	lock_guard<mutex> guard(m);
        	/* Yes we need copy constructor...
        	children.push_back(&file);
        	children_map[file.GetName()] = children[children.size() - 1].get();
        	*/
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
        	if(!GetChild(dir_name))
                return false;

        	lock_guard<mutex> guard(m);
        	children.push_back(make_shared<INodeDirectory>(dir_name, this->GetOwner(), this));
            children_map[dir_name] = children[children.size() - 1].get();
            return false;
        }

        bool INodeDirectory::AddDir(INodeDirectory &dir) {
        	// TODO not finished, need copy constructor -- Need verificaation
            return false;
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
        }

        std::ostream& operator<<(std::ostream& os, const INode& node) {
        	os << node.GetName();
        }

    } // namespace raftfs::server
} // namespace raftfs
