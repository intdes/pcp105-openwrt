#ifndef hyrax_file_h
#define hyrax_file_h

struct file* file_open(const char* path, int flags, int rights);
void file_close(struct file* file);
int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size);
int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size);
int file_feof(struct file* file);
int freadline( struct file *file, char *pzLine, int iLineLength );

#endif //hyrax_file_h

