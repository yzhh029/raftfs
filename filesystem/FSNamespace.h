/*
 * VirtualFS.h
 *  Created on: Apr 22, 2016
 *      Author: huang630
 */

#ifndef VIRTUAL_FS_H_
#define VIRTUAL_FS_H_

#include "../protocol/RaftService.h"
#include <memory>
#include <vector>
#include <deque>
#include <mutex>
#include <string>

#include "INode.h"


namespace raftfs {
    namespace filesystem {

        class FSNamespace {

        public:

            FSNamespace();
            ~FSNamespace();

            /*
             * all abs_dir/file_name/new_file/abs_path should be absolute file path
             * i.e. start with "/"
             */
			static bool ValidatePath(const std::string &path);

            bool MakeDir(const std::string abs_dir, const std::string &owner, bool make_parents);
            bool DeleteDir(const std::string &abs_dir, const std::string &visitor, bool recursive);

            // the parent folder must exist for file operations
            bool CreateFile(const std::string &new_file, const std::string &owner);
            bool RemoveFile(const std::string &file_name, const std::string &visitor);
            bool UpdateFile(const std::string &file_name, const std::string &visitor, protocol::FileInfo new_info);

            bool ExistInode(const std::string &abs_path) const;
            std::vector<std::string> ListDir(const std::string &abs_dir) const;
            bool GetFileInfo(const std::string &filename, protocol::FileInfo &info) const;

			// Print a Log for debugging purposes.
			friend std::ostream& operator<<(std::ostream& os, const FSNamespace& fs);
            void Print() const;

			// Log is not copyable for safety.
			FSNamespace(const FSNamespace&) = delete;
			FSNamespace& operator=(const FSNamespace&) = delete;

        private:

            std::vector<std::string> SplitPath(std::string abs_path) const;
            std::pair<INodeDirectory *, std::string> GetParentFolderAndChild(const std::string &abs_path) const;

        private:
			// Data structures to entry storage
            mutable std::mutex m;

            std::shared_ptr<filesystem::INodeDirectory> root;
        };
    }
}


#endif /* RAFTLOG_H_ */
