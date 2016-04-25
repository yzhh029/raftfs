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

using namespace std;

namespace raftfs {
    namespace filesystem {

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


        bool INode::ValideName(const std::string &name) {
			if (!name.empty() && name[0] == '/')
				return true;
            return false;
		}


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

            // TODO not finished, need copy constructor

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
            // todo
            return false;
        }

        bool INodeDirectory::AddDir(INodeDirectory &dir) {
            // todo
            return false;
        }

        bool INodeDirectory::DeleteChild(const std::string &child_name, bool recursive) {
            // todo
            return false;
        }


        std::ostream& operator<<(std::ostream& os, const INode& node) {
        	os << node.GetName();
        }

    } // namespace raftfs::server
} // namespace raftfs
