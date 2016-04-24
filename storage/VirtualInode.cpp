/*
 * VirtualInode.cpp
 *
 *  Created on: Apr 22, 2016
 *      Author: huang630
 */

#include "VirtualInode.h"
#include <iostream>
#include <mutex>
//#include <algorithm>
#include <vector>
#include <string.h>
#include <cstddef>

using namespace std;

namespace raftfs {
    namespace server {    // FIXME: put this under server first

    	VirtualInode::VirtualInode(const char * new_name, int64_t mode)
                : data(0), isOpen(false) {
    		treenodes.clear();
    		prop = mode;
    		// TODO: should have a "new" operator to return null pointer if wrong mode.
    		if(IS_FILE(prop)) {

    		} else if(IS_DIR(prop)) {

    		} else {
    			// Not assign mode, set as file...
    			prop |= VINODE_FILE;
    		}
    		strcpy(this->name, new_name);
        }

    	VirtualInode::~VirtualInode() {
        	// TODO: shall we lock here?
        	for(auto p: treenodes) {
        		delete p;	// FIXME: remove this if don't need recycle entries.
        	}
        }


        bool VirtualInode::IsEmptyDir() const {
            std::lock_guard<std::mutex> guard(m);
            if(! IS_DIR(prop) )	return false;
            return treenodes.empty();
        }

        bool VirtualInode::IsFile() const {
			//std::lock_guard<std::mutex> guard(m);
			return IS_FILE(prop);
        }

        bool VirtualInode::IsDir() const {
			//std::lock_guard<std::mutex> guard(m);
			return IS_DIR(prop);
        }

        const char * VirtualInode::GetName() const{
        	return name;
        }

        int64_t VirtualInode::GetMode() {
        	return prop;
        }

        std::vector<VirtualInode *> * VirtualInode::GetAllNodes() {
        	return &treenodes;
        }

        bool VirtualInode::ChangeName(char * new_name) {
        	if(strcmp(new_name, "") == 0)
        		return false;	// don't allow empty string
        	if(strlen(new_name) > (VINODE_NAME_LEN-1))
        		return false;

        	strcpy(name, new_name);
        	return true;
        }


		#if(0)
        bool static VirtualInode::IsNodeEmptyDir(VirtualInode * node) {
        	if (node == nullptr) return false;
        	return node->IsEmptyDir();
        }

        bool static VirtualInode::IsNodeDir(VirtualInode * node) {
        	if (node == nullptr) return false;
        	return node->IsDir();
        }

        bool static VirtualInode::IsNodeFile(VirtualInode * node) {
        	if (node == nullptr) return false;
        	return node->IsFile();
        }
		#endif

        VirtualInode* VirtualInode::GetChild(char * name) const {
            lock_guard<mutex> guard(m);
            for(auto p: treenodes) {
            	if(strcmp(p->GetName(), name) == 0) {
            		return p;
            	}
            }
            return nullptr;
        }

        bool VirtualInode::ExistChild(const char * name) {
			lock_guard<mutex> guard(m);
			for(auto p: treenodes) {
				if(strcmp(p->GetName(), name) == 0) {
					return true;
				}
			}
			return false;
        }

        VirtualInode * VirtualInode::ExistChildDir(const char * name) {
			lock_guard<mutex> guard(m);
			for(auto p: treenodes) {
				if(strcmp(p->GetName(), name) == 0) {
					if(p->IsDir())
						return p;	// or we look into next level...
				}
			}
			return nullptr;
        }

        bool VirtualInode::CreateFile(const char * child_name) {
        	// No existing child
        	if(ExistChild(child_name))	return false;
        	// We are a dir
        	if(this->IsFile())	return false;
        	//
        	lock_guard<mutex> guard(m);
        	int64_t newmode = prop | VINODE_FILE & !(VINODE_DIR);
        	VirtualInode * pNew = new VirtualInode(child_name, newmode);
        	treenodes.push_back(pNew);
        	return true;
        }

        bool VirtualInode::CreateDir(const char * child_name) {
        	// No existing child
        	if(ExistChild(child_name))	return false;
        	// We are a dir
        	if(this->IsFile())	return false;
        	//
        	lock_guard<mutex> guard(m);
        	int64_t newmode = prop;		// should be a dir too.
        	VirtualInode * pNew = new VirtualInode(child_name, newmode);
        	treenodes.push_back(pNew);
        	return true;
        }

        bool VirtualInode::DeleteChild(char * child_name) {
			// We are a dir
			if(this->IsFile())	return false;
			// Get lock
			lock_guard<mutex> guard(m);
			// Check if exists child and remove it...
			auto it = treenodes.begin();
			bool found = false;
			for (it = treenodes.begin(); it != treenodes.end(); ++it) {
				if( strcmp( (*it)->GetName(), child_name) == 0) {
					// found
					if( (*it)->IsFile() || (*it)->IsEmptyDir() ) {
						treenodes.erase(it);
						return true;
					} else {
						// not empty
						return false;
					}
				}
			}
			// Not found
			return false;
        }


        std::ostream& operator<<(std::ostream& os, const VirtualInode& node) {
        	os << node.GetName();
        }

    } // namespace raftfs::server
} // namespace raftfs
