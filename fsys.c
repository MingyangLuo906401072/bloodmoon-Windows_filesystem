#include "fsys.h"

int isWhitespaceString(const char *str)
{
    int charCount = 0;
    while (*str != '\0')
    {
        if (charCount >= MAX_CHARS)
        {
            printf("Exceeded maximum character count (%d).\n", MAX_CHARS);
            return 0;
        }

        if (!isspace((unsigned char)*str))
        {
            return 0;
        }
        str++;
        charCount++;
    }

    return 1;
}

void addUserToSystem(struct FileSystem *fs, const char *username,
                     const char *password, enum AuthorityLevel accessLevel)
{
    if (username == NULL || isWhitespaceString(username) || strlen(username) >= MAX_USERNAME_LENGTH ||
        password == NULL || strlen(password) >= MAX_PASSWORD_LENGTH)
    {
        printf("Invalid or too long username or password provided.\n");
        return;
    }

    if (fs->user_count < MAX_USERS)
    {
        strncpy(fs->users[fs->user_count].username, username, MAX_USERNAME_LENGTH - 1);
        strncpy(fs->users[fs->user_count].password, password, MAX_PASSWORD_LENGTH - 1);
        fs->users[fs->user_count].access_level = accessLevel;
        fs->user_count++;
        printf("User added successfully.\n");
    }
    else
    {
        printf("Cannot add more users. Maximum capacity reached.\n");
    }
}

void deleteUserFromSystem(struct FileSystem *fs, const char *username)
{
    if (username == NULL || isWhitespaceString(username) || strlen(username) >= MAX_USERNAME_LENGTH)
    {
        printf("Invalid or too long username provided.\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < fs->user_count; ++i)
    {
        if (strcmp(username, fs->users[i].username) == 0)
        {
            found = 1;
            // Shift elements in the array starting from the found index
            for (int j = i; j < fs->user_count - 1; ++j)
            {
                strcpy(fs->users[j].username, fs->users[j + 1].username);
                strcpy(fs->users[j].password, fs->users[j + 1].password);
                fs->users[j].access_level = fs->users[j + 1].access_level;
            }
            // Clear the last element in the array
            memset(&fs->users[fs->user_count - 1], 0, sizeof(struct User));
            fs->user_count--;
            printf("User '%s' deleted successfully.\n", username);
            break;
        }
    }

    if (!found)
    {
        printf("User '%s' not found in the system.\n", username);
    }
}

struct User loginUser(struct FileSystem *fs, const char *username, const char *password)
{
    if (fs == NULL || username == NULL || password == NULL ||
        isWhitespaceString(username) || isWhitespaceString(password))
    {
        printf("Invalid parameters provided for login.\n");
        struct User emptyUser = {"", "", LOW}; // Return an empty user if login fails
        return emptyUser;
    }

    if (strlen(username) >= MAX_USERNAME_LENGTH || strlen(password) >= MAX_PASSWORD_LENGTH)
    {
        printf("Username or password length exceeds maximum limit.\n");
        struct User emptyUser = {"", "", LOW}; // Return an empty user if login fails
        return emptyUser;
    }

    for (int i = 0; i < fs->user_count; ++i)
    {
        if (strcmp(username, fs->users[i].username) == 0 && strcmp(password, fs->users[i].password) == 0)
        {
            printf("Logged in successfully as %s with level %d.\n", username, fs->users[i].access_level);
            fs->current_user = fs->users[i]; // Set the current user in the file system
            return fs->users[i];
        }
    }
    printf("Login failed. Invalid username or password.\n");
    struct User emptyUser = {"", "", LOW}; // Return an empty user if login fails
    return emptyUser;
}

void resetPassword(struct FileSystem *fs, const char *username, const char *oldPassword, const char *newPassword)
{
    if (fs == NULL || username == NULL || oldPassword == NULL || newPassword == NULL ||
        isWhitespaceString(username) || isWhitespaceString(oldPassword) || isWhitespaceString(newPassword))
    {
        printf("Invalid parameters provided for password reset.\n");
        return;
    }

    if (strlen(username) >= MAX_USERNAME_LENGTH || strlen(newPassword) >= MAX_PASSWORD_LENGTH)
    {
        printf("Username or new password length exceeds maximum limit.\n");
        return;
    }

    for (int i = 0; i < fs->user_count; ++i)
    {
        if (strcmp(username, fs->users[i].username) == 0 && strcmp(oldPassword, fs->users[i].password) == 0)
        {
            strncpy(fs->users[i].password, newPassword, MAX_PASSWORD_LENGTH - 1);
            printf("Password reset successfully for user %s.\n", username);

            // If the current user is resetting their own password, update it in the current_user of FileSystem
            if (strcmp(fs->current_user.username, username) == 0)
            {
                strncpy(fs->current_user.password, newPassword, MAX_PASSWORD_LENGTH - 1);
            }
            return;
        }
    }
    printf("User not found or incorrect old password.\n");
}

void initUser(struct User *user)
{
    if (user != NULL)
    {
        memset(user->username, 0, MAX_USERNAME_LENGTH);
        memset(user->password, 0, MAX_PASSWORD_LENGTH);
        user->access_level = LOW;
    }
}

void initFile(struct File *file)
{
    if (file != NULL)
    {
        memset(file->name, 0, MAX_FILE_NAME_LENGTH);
        memset(file->path, 0, MAX_PATH_LENGTH);
        file->content = NULL;
        file->size = 0;
        file->access = LOW;
    }
}

void initDirectory(struct Directory *dir)
{
    if (dir != NULL)
    {
        memset(dir->name, 0, MAX_FILE_NAME_LENGTH);
        memset(dir->path, 0, MAX_PATH_LENGTH);
        for (int i = 0; i < MAX_FILES; ++i)
        {
            dir->files[i] = NULL;
        }
        dir->file_count = 0;

        for (int i = 0; i < MAX_SUB_DIRS; ++i)
        {
            dir->subdirectories[i] = NULL;
        }
        dir->subdir_count = 0;

        dir->access = LOW;
    }
}

void initFileSystem(struct FileSystem *fs)
{
    if (fs != NULL)
    {
        fs->root = malloc(sizeof(struct Directory));
        if (fs->root == NULL)
        {
            printf("Memory allocation failed for the root directory.\n");
            return;
        }

        initDirectory(fs->root);

        strncpy(fs->root->name, "root", MAX_FILE_NAME_LENGTH - 1);
        fs->root->name[MAX_FILE_NAME_LENGTH - 1] = '\0';
        strcpy(fs->root->path, "~");

        fs->root->subdirectories[0] = malloc(sizeof(struct Directory));
        if (fs->root->subdirectories[0] == NULL)
        {
            printf("Memory allocation failed for the 'home' directory.\n");
            free(fs->root);
            return;
        }

        initDirectory(fs->root->subdirectories[0]);

        strncpy(fs->root->subdirectories[0]->name, "home", MAX_FILE_NAME_LENGTH - 1);
        fs->root->subdirectories[0]->name[MAX_FILE_NAME_LENGTH - 1] = '\0';
        strcpy(fs->root->subdirectories[0]->path, "~/home");

        fs->root->subdir_count = 1;
        fs->current_directory = fs->root->subdirectories[0];

        fs->user_count = 0;
        fs->current_user.access_level = LOW;
        strcpy(fs->current_user.username, "guest");

        for (int i = 0; i < MAX_USERS; ++i)
        {
            initUser(&(fs->users[i]));
        }

        strcpy(fs->users[fs->user_count].username, "admin");
        strcpy(fs->users[fs->user_count].password, "YAwC4@admin1r2#3");
        fs->users[fs->user_count].access_level = HIGHEST;
        fs->user_count++;
    }
}

struct File *createFile(const char *name, const char *path)
{
    if (name == NULL || isWhitespaceString(name) || strlen(name) >= MAX_FILE_NAME_LENGTH)
    {
        printf("Invalid or too long file name provided.\n");
        return NULL;
    }

    // Check if the resulting path length exceeds the maximum limit
    if (strlen(path) >= MAX_PATH_LENGTH)
    {
        printf("Path length exceeds maximum limit.\n");
        return NULL;
    }

    struct File *newFile = malloc(sizeof(struct File));
    if (newFile == NULL)
    {
        printf("Memory allocation failed for file creation.\n");
        return NULL;
    }

    newFile->content = malloc(MAX_CONTENT_SIZE * sizeof(char));
    if (newFile->content == NULL)
    {
        printf("Memory allocation failed for file content.\n");
        free(newFile);
        return NULL;
    }

    initFile(newFile);

    snprintf(newFile->name, MAX_FILE_NAME_LENGTH, "%s", name);
    snprintf(newFile->path, MAX_PATH_LENGTH, "%s/%s", path, name);

    newFile->size = 0;
    return newFile;
}

void createFileInDir(struct FileSystem *fs, const char *path, const char *name)
{
    if (name == NULL || isWhitespaceString(name) || strlen(name) >= MAX_FILE_NAME_LENGTH)
    {
        printf("Invalid or too long file name provided.\n");
        return;
    }

    // Check if the resulting path length exceeds the maximum limit
    if (strlen(path) >= MAX_PATH_LENGTH)
    {
        printf("Path length exceeds maximum limit.\n");
        return;
    }

    struct Directory *dir = goTo(fs, path);
    if (dir != NULL)
    {
        if (dir->file_count >= MAX_FILES)
        {
            printf("File limit reached in directory. Cannot create more files.\n");
            return;
        }

        // Check if the file name already exists at the same level
        for (int i = 0; i < dir->file_count; ++i)
        {
            if (dir->files[i]->content != NULL && strcmp(dir->files[i]->name, name) == 0)
            {
                printf("File with the same name already exists at this level.\n");
                return;
            }
        }

        // Check if the resulting path length with the new file name exceeds the maximum limit
        if (strlen(dir->path) >= MAX_PATH_LENGTH)
        {
            printf("Path length exceeds maximum limit.\n");
            return;
        }

        // Find the correct position to insert the new file based on alphabetical order
        int pos = dir->file_count;
        while (pos > 0 && strcmp(name, dir->files[pos - 1]->name) < 0)
        {
            pos--;
        }

        // Shift files to make space for the new file
        for (int i = dir->file_count; i > pos; --i)
        {
            dir->files[i] = dir->files[i - 1];
        }

        // Create the new file and insert it at the appropriate position
        dir->files[pos] = createFile(name, dir->path);
        if (dir->files[pos] != NULL)
        {
            dir->file_count++;
        }
    }
    else
    {
        printf("Directory not found at path: %s\n", path);
    }
}

void createDirectory(struct FileSystem *fs, const char *path, const char *name)
{
    if (name == NULL || isWhitespaceString(name))
    {
        printf("Invalid directory name provided.\n");
        return;
    }

    if (strlen(name) >= MAX_FILE_NAME_LENGTH)
    {
        printf("Directory name length exceeds maximum limit.\n");
        return;
    }

    if (path == NULL)
    {
        printf("Invalid path provided for directory creation.\n");
        return;
    }

    if (strlen(path) >= MAX_PATH_LENGTH)
    {
        printf("Path length exceeds maximum limit.\n");
        return;
    }

    struct Directory *parentDir = goTo(fs, path);
    if (parentDir != NULL)
    {
        if (parentDir->subdir_count >= MAX_SUB_DIRS)
        {
            printf("Subdirectory limit reached in the parent directory. Cannot create more subdirectories.\n");
            return;
        }

        // Check if the directory already exists in the parent directory
        for (int i = 0; i < parentDir->subdir_count; ++i)
        {
            if (strcmp(parentDir->subdirectories[i]->name, name) == 0)
            {
                printf("Directory '%s' already exists in path: %s\n", name, path);
                return;
            }
        }

        // Create a new directory
        struct Directory *newDir = malloc(sizeof(struct Directory));
        if (newDir == NULL)
        {
            printf("Memory allocation failed for directory creation.\n");
            return;
        }

        initDirectory(newDir);

        strncpy(newDir->name, name, MAX_FILE_NAME_LENGTH - 1);
        newDir->name[MAX_FILE_NAME_LENGTH - 1] = '\0'; // Ensure null-terminated string

        // Constructing the new directory's path correctly
        char newPath[MAX_PATH_LENGTH];
        snprintf(newPath, MAX_PATH_LENGTH, "%s/%s", parentDir->path, name);

        strncpy(newDir->path, newPath, MAX_PATH_LENGTH - 1);
        newDir->path[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-terminated string

        int insertIdx = 0;

        // Find the correct position to insert the new directory based on alphabetical order
        while (insertIdx < parentDir->subdir_count && strcmp(newDir->name, parentDir->subdirectories[insertIdx]->name) > 0)
        {
            insertIdx++;
        }

        // Shift directories to make space for the new directory
        for (int i = parentDir->subdir_count; i > insertIdx; --i)
        {
            parentDir->subdirectories[i] = parentDir->subdirectories[i - 1];
        }

        // Insert the new directory at the appropriate position
        parentDir->subdirectories[insertIdx] = newDir;
        parentDir->subdir_count++;

        printf("Directory '%s' created at path: %s\n", name, newDir->path);
    }
    else
    {
        printf("Parent directory not found at path: %s\n", path);
    }
}

void writeFile(struct FileSystem *fs, const char *filePath, const char *fileName, const char *content)
{
    if (fs == NULL || filePath == NULL || fileName == NULL || content == NULL ||
        isWhitespaceString(filePath) || isWhitespaceString(fileName))
    {
        printf("Invalid parameters provided for file writing.\n");
        return;
    }

    size_t pathLength = strlen(filePath);
    size_t fileNameLength = strlen(fileName);

    // Check for maximum path and file name lengths
    if (pathLength >= MAX_PATH_LENGTH || fileNameLength >= MAX_FILE_NAME_LENGTH)
    {
        printf("Path or file name length exceeds maximum limit.\n");
        return;
    }

    struct Directory *dir = goTo(fs, filePath);

    if (dir != NULL)
    {
        // Check if the file already exists in the directory
        int fileExists = 0;
        for (int i = 0; i < dir->file_count; ++i)
        {
            if (strcmp(dir->files[i]->name, fileName) == 0)
            {
                sprintf(dir->files[i]->content, "%s", content);
                dir->files[i]->size = strlen(dir->files[i]->content);
                printf("Content written to file '%s'.\n", fileName);
                fileExists = 1;
                break;
            }
        }

        // If the file doesn't exist, create it
        if (!fileExists)
        {
            // Check if the directory has space for a new file
            if (dir->file_count >= MAX_FILES)
            {
                printf("Maximum file count reached in directory '%s'. Cannot create a new file.\n", filePath);
                return;
            }

            // Create a new file in the directory
            struct File *newFile = (struct File *)malloc(sizeof(struct File));
            if (newFile == NULL)
            {
                printf("Memory allocation failed for file creation.\n");
                return;
            }

            strncpy(newFile->name, fileName, MAX_FILE_NAME_LENGTH - 1);
            newFile->name[MAX_FILE_NAME_LENGTH - 1] = '\0'; // Ensure null-terminated string
            strncpy(newFile->path, filePath, MAX_PATH_LENGTH - 1);
            newFile->path[MAX_PATH_LENGTH - 1] = '\0'; // Ensure null-terminated string

            newFile->content = (char *)malloc((strlen(content) + 1) * sizeof(char));
            if (newFile->content == NULL)
            {
                free(newFile);
                printf("Memory allocation failed for file content.\n");
                return;
            }

            strcpy(newFile->content, content);
            newFile->size = strlen(content);
            dir->files[dir->file_count++] = newFile;

            printf("File '%s' created and content written.\n", fileName);
        }
    }
    else
    {
        printf("Directory not found at path: %s\n", filePath);
    }
}

void readFile(struct FileSystem *fs, const char *filePath, const char *fileName)
{
    if (fs == NULL || filePath == NULL || fileName == NULL || isWhitespaceString(filePath) || isWhitespaceString(fileName))
    {
        printf("Invalid parameters provided for file reading.\n");
        return;
    }

    size_t pathLength = strlen(filePath);
    size_t fileNameLength = strlen(fileName);

    // Check for maximum path and file name lengths
    if (pathLength >= MAX_PATH_LENGTH || fileNameLength >= MAX_FILE_NAME_LENGTH)
    {
        printf("Path or file name length exceeds maximum limit.\n");
        return;
    }

    struct Directory *dir = goTo(fs, filePath);

    if (dir != NULL)
    {
        if (dir->file_count <= 0)
        {
            printf("No files found in directory '%s'.\n", filePath);
            return;
        }

        // Check if the file exists in the directory
        for (int i = 0; i < dir->file_count; ++i)
        {
            if (strcmp(dir->files[i]->name, fileName) == 0)
            {
                printf("Content of file '%s':\n%s\n", fileName, dir->files[i]->content);
                return;
            }
        }

        printf("File '%s' not found in directory '%s'.\n", fileName, filePath);
    }
    else
    {
        printf("Directory not found at path: %s\n", filePath);
    }
}

void deleteFileAtPath(struct FileSystem *fs, const char *path, const char *fileName)
{
    if (fs == NULL || path == NULL || fileName == NULL || isWhitespaceString(path) || isWhitespaceString(fileName))
    {
        printf("Invalid parameters provided for file deletion.\n");
        return;
    }

    size_t pathLength = strlen(path);
    size_t fileNameLength = strlen(fileName);

    // Check for maximum path and file name lengths
    if (pathLength >= MAX_PATH_LENGTH || fileNameLength >= MAX_FILE_NAME_LENGTH)
    {
        printf("Path or file name length exceeds maximum limit.\n");
        return;
    }

    struct Directory *dir = goTo(fs, path);

    if (dir != NULL)
    {
        if (dir->file_count <= 0)
        {
            printf("No files found in directory '%s'.\n", path);
            return;
        }

        int left = 0, right = dir->file_count - 1;
        int mid, comparison;

        while (left <= right)
        {
            mid = left + (right - left) / 2;
            comparison = strcmp(dir->files[mid]->name, fileName);

            if (comparison == 0)
            {
                // Free file content
                if (dir->files[mid]->content != NULL)
                {
                    free(dir->files[mid]->content);
                }

                // Free file structure
                free(dir->files[mid]);

                // Rearrange the file array
                dir->files[mid] = dir->files[dir->file_count - 1];
                dir->files[dir->file_count - 1] = NULL;
                dir->file_count--;

                printf("File '%s' deleted from directory '%s'.\n", fileName, dir->name);
                return;
            }

            if (comparison < 0)
            {
                right = mid - 1;
            }
            else
            {
                left = mid + 1;
            }
        }
        printf("File '%s' not found in directory '%s'.\n", fileName, path);
    }
    else
    {
        printf("Directory not found at path: %s\n", path);
    }
}

// Helper function for binary search to find directory index
int binarySearchDir(struct Directory *dirs[], int l, int r, const char *name)
{
    while (l <= r)
    {
        int mid = l + (r - l) / 2;
        int cmp = strcmp(dirs[mid]->name, name);

        if (cmp == 0)
        {
            return mid;
        }
        else if (cmp < 0)
        {
            l = mid + 1;
        }
        else
        {
            r = mid - 1;
        }
    }
    return -1;
}

void deleteDirectoryAtPath(struct FileSystem *fs, const char *path)
{
    if (fs == NULL || path == NULL || isWhitespaceString(path))
    {
        printf("Invalid parameters provided. Cannot delete directory.\n");
        return;
    }

    // Check for potential buffer overflow in the path
    if (strlen(path) >= MAX_PATH_LENGTH)
    {
        printf("Path length exceeds maximum limit.\n");
        return;
    }

    struct Directory *parentDir = goTo(fs, path);

    if (parentDir != NULL)
    {
        int idx = binarySearchDir(parentDir->subdirectories, 0, parentDir->subdir_count - 1, path);

        if (idx != -1)
        {
            struct Directory *dirToDelete = parentDir->subdirectories[idx];

            if (dirToDelete->file_count > 0)
            {
                // Delete files within the directory
                for (int i = 0; i < dirToDelete->file_count; ++i)
                {
                    deleteFileAtPath(fs, dirToDelete->path, dirToDelete->files[i]->name);
                }
            }

            if (dirToDelete->subdir_count > 0)
            {
                // Delete subdirectories recursively
                for (int i = 0; i < dirToDelete->subdir_count; ++i)
                {
                    deleteDirectoryAtPath(fs, dirToDelete->subdirectories[i]->path);
                }
            }

            printf("Directory '%s' at path '%s' deleted.\n", dirToDelete->name, dirToDelete->path);
            free(dirToDelete);

            // Shift elements after deletion
            for (int i = idx; i < parentDir->subdir_count - 1; ++i)
            {
                parentDir->subdirectories[i] = parentDir->subdirectories[i + 1];
            }
            parentDir->subdirectories[parentDir->subdir_count - 1] = NULL;
            parentDir->subdir_count--;
        }
        else
        {
            printf("Directory not found at path: %s\n", path);
        }
    }
    else
    {
        printf("Parent directory not found at path: %s\n", path);
    }
}

void moveDirectoryAtPath(struct FileSystem *fs, const char *sourcePath, const char *destinationPath)
{
    if (fs == NULL || sourcePath == NULL || destinationPath == NULL ||
        isWhitespaceString(sourcePath) || isWhitespaceString(destinationPath))
    {
        printf("Invalid parameters provided. Cannot move directory.\n");
        return;
    }

    // Check for potential buffer overflow in the paths
    if (strlen(sourcePath) >= MAX_PATH_LENGTH || strlen(destinationPath) >= MAX_PATH_LENGTH)
    {
        printf("Path length exceeds maximum limit.\n");
        return;
    }

    struct Directory *sourceDir = goTo(fs, sourcePath);
    struct Directory *destinationDir = goTo(fs, destinationPath);

    if (sourceDir != NULL && destinationDir != NULL)
    {
        int index = binarySearchDir(sourceDir->subdirectories, 0, sourceDir->subdir_count - 1, destinationPath);

        if (index != -1)
        {
            // Move files to the destination directory
            for (int i = 0; i < sourceDir->file_count; ++i)
            {
                if (destinationDir->file_count >= MAX_FILES)
                {
                    printf("Destination directory has reached maximum file count. Cannot move all files.\n");
                    return;
                }

                char newPath[MAX_PATH_LENGTH];
                snprintf(newPath, MAX_PATH_LENGTH, "%s/%s", destinationPath, sourceDir->files[i]->name);
                snprintf(sourceDir->files[i]->path, MAX_PATH_LENGTH, "%s", newPath);
                destinationDir->files[destinationDir->file_count++] = sourceDir->files[i];
                sourceDir->files[i] = NULL;
            }
            sourceDir->file_count = 0;

            // Move subdirectories to the destination directory
            for (int i = 0; i < sourceDir->subdir_count; ++i)
            {
                if (destinationDir->subdir_count >= MAX_SUB_DIRS)
                {
                    printf("Destination directory has reached maximum subdirectory count. Cannot move all subdirectories.\n");
                    return;
                }

                char newPath[MAX_PATH_LENGTH];
                snprintf(newPath, MAX_PATH_LENGTH, "%s/%s", destinationPath, sourceDir->subdirectories[i]->name);
                snprintf(sourceDir->subdirectories[i]->path, MAX_PATH_LENGTH, "%s", newPath);
                destinationDir->subdirectories[destinationDir->subdir_count++] = sourceDir->subdirectories[i];
                sourceDir->subdirectories[i] = NULL;
            }
            sourceDir->subdir_count = 0;

            printf("Directory '%s' moved to '%s'.\n", sourceDir->name, destinationDir->name);
        }
        else
        {
            printf("Source directory '%s' not found or not a subdirectory of '%s'. Cannot move.\n", sourceDir->name, destinationDir->name);
        }
    }
    else
    {
        printf("Invalid source or destination directory. Cannot move.\n");
    }
}

void moveFileAtPath(struct FileSystem *fs, const char *sourcePath, const char *destinationPath, const char *fileName)
{
    if (fs == NULL || sourcePath == NULL || destinationPath == NULL || fileName == NULL ||
        isWhitespaceString(sourcePath) || isWhitespaceString(destinationPath) || isWhitespaceString(fileName))
    {
        printf("Invalid parameters provided. Cannot move file.\n");
        return;
    }

    // Check for potential buffer overflow in the paths and fileName
    if (strlen(sourcePath) >= MAX_PATH_LENGTH || strlen(destinationPath) >= MAX_PATH_LENGTH || strlen(fileName) >= MAX_FILE_NAME_LENGTH)
    {
        printf("Path or file name length exceeds maximum limit.\n");
        return;
    }

    struct Directory *sourceDir = goTo(fs, sourcePath);
    struct Directory *destinationDir = goTo(fs, destinationPath);

    if (sourceDir != NULL && destinationDir != NULL)
    {
        // Binary search in source directory to find the file
        int left = 0;
        int right = sourceDir->file_count - 1;
        int found = 0;

        while (left <= right)
        {
            int mid = left + (right - left) / 2;
            int comparison = strcmp(sourceDir->files[mid]->name, fileName);

            if (comparison == 0)
            {
                struct File *fileToMove = sourceDir->files[mid];

                char newPath[MAX_PATH_LENGTH];
                snprintf(newPath, MAX_PATH_LENGTH, "%s/%s", destinationDir->path, fileToMove->name);

                // Check if the destination directory already contains a file with the same name
                int fileExists = 0;
                for (int i = 0; i < destinationDir->file_count; ++i)
                {
                    if (strcmp(destinationDir->files[i]->name, fileToMove->name) == 0)
                    {
                        printf("File '%s' already exists in '%s'. Cannot move.\n", fileName, destinationDir->name);
                        return;
                    }
                }

                snprintf(fileToMove->path, MAX_PATH_LENGTH, "%s", newPath);

                // Add file to the destination directory
                destinationDir->files[destinationDir->file_count++] = fileToMove;
                sourceDir->files[mid] = NULL;
                sourceDir->file_count--;

                printf("File '%s' moved from '%s' to '%s'.\n", fileName, sourceDir->name, destinationDir->name);
                found = 1;
                break;
            }

            if (comparison < 0)
            {
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

        if (!found)
        {
            printf("File '%s' not found in '%s'. Cannot move.\n", fileName, sourceDir->name);
        }
    }
    else
    {
        printf("Invalid parameters provided. Cannot move file.\n");
    }
}

void searchFileInPath(struct FileSystem *fs, const char *path, const char *fileName)
{
    if (fs == NULL || path == NULL || fileName == NULL || isWhitespaceString(path) || isWhitespaceString(fileName))
    {
        printf("Invalid parameters provided for file search.\n");
        return;
    }

    // Check for potential buffer overflow in the path and fileName
    if (strlen(path) >= MAX_PATH_LENGTH || strlen(fileName) >= MAX_FILE_NAME_LENGTH)
    {
        printf("Path or file name length exceeds maximum limit.\n");
        return;
    }

    struct Directory *currentDir = goTo(fs, path);

    if (currentDir != NULL)
    {
        int left = 0;
        int right = currentDir->file_count - 1;
        int found = 0;

        // Binary search in current directory
        while (left <= right)
        {
            int mid = left + (right - left) / 2;
            int comparison = strcmp(currentDir->files[mid]->name, fileName);

            if (comparison == 0)
            {
                printf("File '%s' found at path: %s\n", fileName, currentDir->files[mid]->path);
                found = 1;
                break;
            }

            if (comparison < 0)
            {
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

        // Recursively search in subdirectories
        for (int i = 0; i < currentDir->subdir_count; ++i)
        {
            searchFileInPath(fs, currentDir->subdirectories[i]->path, fileName);
        }

        if (!found)
        {
            printf("File '%s' not found in path: %s\n", fileName, path);
        }
    }
    else
    {
        printf("Invalid parameters provided for file search.\n");
    }
}

struct Directory *goTo(struct FileSystem *fs, const char *path)
{
    if (fs == NULL || path == NULL || isWhitespaceString(path))
    {
        printf("Invalid parameters provided for directory navigation.\n");
        return NULL;
    }

    // Check for potential buffer overflow in the path
    if (strlen(path) >= MAX_PATH_LENGTH)
    {
        printf("Path length exceeds maximum limit.\n");
        return NULL;
    }

    if (path[0] != '/')
    {
        printf("Path must start with '/'.\n");
        return NULL;
    }

    char *inputPath = path;

    if (strcmp(path, ".") == 0)
    {
        inputPath = getCurrentDirectoryPath(fs);
    }

    struct Directory *currentDir = fs->root;
    char currentPath[MAX_PATH_LENGTH] = ""; // Track the current path

    // Traversing through directories
    char *token = strtok(inputPath, "/");

    while (token != NULL)
    {
        // Check for individual directory name length
        size_t dirNameLength = strlen(token);
        if (dirNameLength >= MAX_FILE_NAME_LENGTH)
        {
            printf("Directory name '%s' length exceeds maximum limit.\n", token);
            return NULL;
        }

        int found = 0;

        // Binary search in sorted subdirectories
        int low = 0;
        int high = currentDir->subdir_count - 1;
        while (low <= high)
        {
            int mid = low + (high - low) / 2;
            int comparison = strcmp(currentDir->subdirectories[mid]->name, token);

            if (comparison == 0)
            {
                currentDir = currentDir->subdirectories[mid];
                found = 1;

                // Update the current path
                snprintf(currentPath, MAX_PATH_LENGTH, "%s/%s", currentPath, token);
                break;
            }
            else if (comparison < 0)
            {
                low = mid + 1;
            }
            else
            {
                high = mid - 1;
            }
        }

        if (!found)
        {
            printf("Directory '%s' not found in path '%s'.\n", token, path);
            return NULL;
        }

        token = strtok(NULL, "/");
    }

    fs->current_directory = currentDir;

    // Update the path correctly
    snprintf(fs->current_directory->path, MAX_PATH_LENGTH, "~%s", currentPath);
    return fs->current_directory;
}

void displayCurrentDirectory(struct FileSystem *fs, const char *path)
{
    if (fs == NULL || path == NULL)
    {
        printf("Invalid parameters provided for directory display.\n");
        return;
    }

    // Check for potential buffer overflow in the path
    if (strlen(path) >= MAX_PATH_LENGTH)
    {
        printf("Path length exceeds maximum limit.\n");
        return;
    }

    struct Directory *currentDir = goTo(fs, path);

    if (currentDir != NULL)
    {
        printf("Current Directory: %s\n", currentDir->name);
        printf("Path: %s\n", currentDir->path);
        printf("Files and Directories in the first level:\n");

        // Display files in the directory if they exist
        printf("Files:\n");
        if (currentDir->file_count > 0)
        {
            for (int i = 0; i < currentDir->file_count; ++i)
            {
                if (currentDir->files[i] != NULL)
                {
                    printf("File %d: %s\n", i + 1, currentDir->files[i]->name);
                }
            }
        }
        else
        {
            printf("No files in this directory.\n");
        }

        // Display directories in the directory if they exist
        printf("Directories:\n");
        if (currentDir->subdir_count > 0)
        {
            for (int i = 0; i < currentDir->subdir_count; ++i)
            {
                if (currentDir->subdirectories[i] != NULL)
                {
                    printf("Directory %d: %s\n", i + 1, currentDir->subdirectories[i]->name);
                }
            }
        }
        else
        {
            printf("No directories in this directory.\n");
        }
    }
    else
    {
        printf("Invalid directory or path provided for display.\n");
    }
}

void displayFileInDirectory(struct FileSystem *fs, const char *path, const char *fileName)
{
    if (fs == NULL || path == NULL || fileName == NULL || isWhitespaceString(fileName))
    {
        printf("Invalid parameters provided for file display.\n");
        return;
    }

    // Check the length of fileName and path
    size_t fileNameLength = strlen(fileName);
    size_t pathLength = strlen(path);
    if (fileNameLength == 0 || fileNameLength >= MAX_FILE_NAME_LENGTH || pathLength >= MAX_PATH_LENGTH)
    {
        printf("Invalid file name or path length.\n");
        return;
    }

    struct Directory *currentDir = goTo(fs, path);

    if (currentDir != NULL)
    {
        int found = 0;
        for (int i = 0; i < currentDir->file_count; ++i)
        {
            if (currentDir->files[i] != NULL && strncmp(currentDir->files[i]->name, fileName, MAX_FILE_NAME_LENGTH) == 0)
            {
                printf("File Name: %s\n", currentDir->files[i]->name);
                printf("File Path: %s\n", currentDir->files[i]->path);
                printf("File Size: %d bytes\n", currentDir->files[i]->size);
                printf("File Content:\n%s\n", currentDir->files[i]->content);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            printf("File '%s' not found in the directory.\n", fileName);
        }
    }
    else
    {
        printf("Directory not found or invalid path provided.\n");
    }
}

void changeFileAccessLevel(struct FileSystem *fs, const char *path, const char *fileName, enum AuthorityLevel newAccessLevel)
{
    if (fs == NULL || path == NULL || fileName == NULL || isWhitespaceString(path) || isWhitespaceString(fileName))
    {
        printf("Invalid input parameter detected.\n");
        return;
    }

    // Check for potential buffer overflow in the file path and file name
    if (strlen(path) + strlen(fileName) + 1 >= MAX_PATH_LENGTH || strlen(fileName) >= MAX_FILE_NAME_LENGTH)
    {
        printf("File path or file name length exceeds maximum limit.\n");
        return;
    }

    // Check if the newAccessLevel is a valid AuthorityLevel value
    if (newAccessLevel < LOW || newAccessLevel > HIGHEST)
    {
        printf("Invalid AuthorityLevel specified.\n");
        return;
    }

    char filePath[MAX_PATH_LENGTH];
    snprintf(filePath, MAX_PATH_LENGTH, "%s/%s", path, fileName);

    struct Directory *fileDir = goTo(fs, filePath);
    if (fileDir != NULL && fileDir->file_count > 0)
    {
        for (int i = 0; i < fileDir->file_count; ++i)
        {
            if (fileDir->files[i] != NULL && strcmp(fileDir->files[i]->name, fileName) == 0)
            {
                fileDir->files[i]->access = newAccessLevel;
                printf("File access level changed successfully.\n");
                return;
            }
        }
        printf("File '%s' not found at path '%s'.\n", fileName, path);
    }
    else
    {
        printf("File not found or invalid path provided.\n");
    }
}

void changeDirectoryAccessLevel(struct FileSystem *fs, const char *dirPath, enum AuthorityLevel newAccessLevel)
{
    if (fs == NULL || dirPath == NULL)
    {
        printf("Invalid parameter detected.\n");
        return;
    }

    // Check for potential buffer overflow in the directory path
    if (strlen(dirPath) >= MAX_PATH_LENGTH)
    {
        printf("Directory path length exceeds maximum limit.\n");
        return;
    }

    // Check if the newAccessLevel is a valid AuthorityLevel value
    if (newAccessLevel < LOW || newAccessLevel > HIGHEST)
    {
        printf("Invalid AuthorityLevel specified.\n");
        return;
    }

    struct Directory *targetDir = goTo(fs, dirPath);
    if (targetDir != NULL && targetDir->subdir_count > 0)
    {
        for (int i = 0; i < targetDir->subdir_count; ++i)
        {
            if (targetDir->subdirectories[i] != NULL)
            {
                targetDir->subdirectories[i]->access = newAccessLevel;
            }
        }
        printf("Directory access level changed successfully.\n");
    }
    else
    {
        printf("Directory not found or invalid path provided.\n");
    }
}

char *getCurrentDirectoryPath(struct FileSystem *fs)
{
    if (fs == NULL)
    {
        printf("Invalid file system provided.\n");
        return NULL;
    }

    struct Directory *currentDir = fs->current_directory;
    if (currentDir == NULL)
    {
        printf("Current directory not set in the file system.\n");
        return NULL;
    }

    return currentDir->path;
}

struct File *getFileInDirectory(struct FileSystem *fs, const char *path, const char *fileName)
{
    if (fs == NULL || path == NULL || fileName == NULL)
    {
        printf("Invalid parameter detected.\n");
        return NULL;
    }

    // Check for potential buffer overflow in path and fileName
    size_t pathLength = strlen(path);
    size_t fileNameLength = strlen(fileName);

    if (pathLength >= MAX_PATH_LENGTH || fileNameLength >= MAX_FILE_NAME_LENGTH)
    {
        printf("Path or file name length exceeds maximum limit.\n");
        return NULL;
    }

    struct Directory *dir = goTo(fs, path);

    if (dir != NULL)
    {
        for (int i = 0; i < dir->file_count; ++i)
        {
            if (dir->files[i] != NULL && strncmp(dir->files[i]->name, fileName, MAX_FILE_NAME_LENGTH) == 0)
            {
                return dir->files[i];
            }
        }
    }

    printf("File '%s' not found in the directory '%s'.\n", fileName, path);
    return NULL;
}

char *getCurrentUser(struct FileSystem *fs)
{
    if (fs == NULL)
    {
        return NULL;
    }

    return fs->current_user.username;
}
