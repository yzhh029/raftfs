/*
 * VirtualInode.h
 *  Created on: Apr 22, 2016
 *      Author: huang630
 */

#ifndef VIRTUAL_INODE_H_
#define VIRTUAL_INODE_H_

#include <memory>
#include <vector>
//#include <deque>
#include <mutex>
#include <string.h>
#include <string>

#define VINODE_FILE		0x00000001
#define VINODE_DIR		0x00000002

#define IS_FILE(a) ((a & VINODE_FILE) && !(a & VINODE_DIR))

#define IS_DIR(a) ((a & VINODE_DIR) && !(a & VINODE_FILE))

#define VINODE_NAME_LEN		128

namespace raftfs {
    namespace  server {		// FIXME: Temporary put this under server...

        class VirtualInode {

        public:
        	VirtualInode(const char * name, int64_t mode);
            ~VirtualInode();

            bool IsEmptyDir() const ;
            bool IsFile() const;
            bool IsDir() const;
            bool ChangeName(char * new_name);

            #if(0)
            bool static IsNodeEmptyDir(VirtualInode * node);
            bool static IsNodeDir(VirtualInode * node);
            bool static IsNodeFile(VirtualInode * node);
			#endif

            const char * GetName() const;
            VirtualInode* GetChild(char * name) const;
            int64_t GetMode();
            std::vector<VirtualInode *> * GetAllNodes();

            /**
             * Check if a child exists
             */
            bool ExistChild(const char * child_name);
            VirtualInode * ExistChildDir(const char * child_name);

			/**
			 * Create a child node.
			 */
            // Currently use parent's mode / prop.
            bool CreateFile(const char * child_name);
            bool CreateDir(const char * child_name);
			//bool CreateFile(char * child_name, int64_t mode);
			//bool CreateDir(char * child_name, int64_t mode);

			/**
			 * Delete a child node.
			 */
			bool DeleteChild(char * child_name);

			// Print a Log for debugging purposes.
			friend std::ostream& operator<<(std::ostream& os, const VirtualInode& node);

			// Inode is not copyable for safety.
			VirtualInode(const VirtualInode&) = delete;
			VirtualInode& operator=(const VirtualInode&) = delete;

        private:
			// Data structures of each inode
			bool isOpen;
			char name[VINODE_NAME_LEN];
			// a map<name[128], VirtualInode> might be better...
			std::vector<VirtualInode *> treenodes;
			int64_t data;		// a virtual data
			int64_t prop;		// property / access / file / dir of a node.
			//
			// TODO: add owner / group / etc. if necessary.
			/*
			=> File type (executable, block special etc)
			=> Permissions (read, write etc)
			=> Owner
			=> Group
			=> File Size
			=> File access, change and modification time
			=> File deletion time
			=> Number of links (soft/hard)
			=> Extended attribute such as append only or no one can delete file including root user (immutability)
			=> Access Control List (ACLs)
			*/
			std::string creator;
			int64_t	create_time;
			int64_t file_size;
            mutable std::mutex m;

        };
    }
}


#endif /* VIRTUAL_INODE_H_ */
