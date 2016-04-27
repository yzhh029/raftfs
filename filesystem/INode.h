/*
 * VirtualInode.h
 *  Created on: Apr 22, 2016
 *      Author: huang630
 */

#ifndef VIRTUAL_INODE_H_
#define VIRTUAL_INODE_H_

#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <string.h>
#include <string>

#include "../protocol/Filesystem_types.h"

#define VINODE_FILE		0x00000001
#define VINODE_DIR		0x00000002

#define IS_FILE(a) ((a & VINODE_FILE) && !(a & VINODE_DIR))

#define IS_DIR(a) ((a & VINODE_DIR) && !(a & VINODE_FILE))

#define VINODE_NAME_LEN		128

namespace raftfs {
    namespace  filesystem {

        class INode {

        public:

            using TimePoint = std::chrono::steady_clock::time_point;

            INode() = default;
        	INode(const std::string _name, const std::string _owner, INode *_parent);
            virtual ~INode();

            virtual bool IsFile() const = 0;

            virtual bool IsDir() const = 0;

            virtual bool IsRoot() const = 0;

            static bool ValidName(const std::string& name);

            const std::string GetName() const { return name; }
            const std::string GetOwner() const { return owner; }

            bool ChangeName(const std::string &new_name) {
                if (ValidName(new_name)) {
                    name = new_name;
                    return true;
                }
                return false;
            }

            INode* GetParent() const { return parent; }
            void SetParent(INode* _p) { parent = _p;}

			// Print a Log for debugging purposes.
			friend std::ostream& operator<<(std::ostream& os, const INode& node);

        protected:
            // basic inode info
            std::string name;
            std::string owner;

            const TimePoint create_time;
            TimePoint modification_time;
            TimePoint access_time;

            // pointer to parent
            INode* parent;

            mutable std::mutex m;

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

        };


		class INodeFile : public INode {
        public:

            INodeFile(const std::string _name, INode *_parent);
            INodeFile(const std::string _name, const std::string _owner, INode *_parent);

            typedef int DataType;

            virtual bool IsFile() const override {
                return true;
            }

            virtual bool IsDir() const override {
                return false;
            }

            virtual bool IsRoot() const override {
                return false;
            }

            protocol::FileInfo ToFileInfo() const;

            friend std::ostream& operator<<(std::ostream& os, const INodeFile& node);

        protected:
            int64_t size;
            DataType data;
        };


        class INodeDirectory : public INode {
        public:

            INodeDirectory() = default;
            INodeDirectory(const std::string _name, const std::string _owner, INode * _parent);
            bool IsEmpty() const {
                std::lock_guard<std::mutex> lock(m);
                return children.empty();
            }

            virtual bool IsFile() const override {
                return false;
            }

            virtual bool IsDir() const override {
                return true;
            }
            // only root can has empty name
            virtual bool IsRoot() const override {
                return name.empty();
            }

            std::vector<std::string> ListDirName() const;
            std::vector<INode*> ListDirINode() const;

            /**
             * get a child INode
             * if not exist, return nullptr
             */
            INode* GetChild(const std::string& child_name) const;

            /**
             * Create a child node.
             */
            // Currently use parent's mode / prop.
            bool CreateFile(const std::string &file_name);
            bool AddFile(INodeFile &file);
            bool CreateDir(std::string &dir_name);
            bool AddDir(INodeDirectory &dir);
            /**
			 * Delete a child node.
			 */
            bool DeleteChild(const std::string &child_name, bool recursive);
            bool DeleteAllChild();	// default recursive...

            friend std::ostream& operator<<(std::ostream& os, const INodeDirectory& node);

        protected:
            // files and subdirectories in this folder
            std::vector<std::shared_ptr<INode> > children;
            std::unordered_map<std::string, INode *> children_map;
        };
    }
}


#endif /* VIRTUAL_INODE_H_ */
