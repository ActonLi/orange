#include "orange_file.h"
#include "orange.h"

#define ORANGE_FILE_LINE_MAX 1024

int orange_file_fread_line(char* line, int size, FILE* fp)
{
	int length = 0;

	if (fgets(line, size, fp) != NULL) {
		length = strlen(line);
		if (line[length - 1] == '\n' && line[length - 2] == '\r') {
			line[length - 2] = '\0';
			length -= 2;
		} else {
			line[length - 1] = '\0';
			length--;
		}
	}

	return length;
}

int orange_file_fwrite_line(char* line, FILE* fp)
{
	int ret = -1;

	ret = fputs(line, fp);
	if (ret == EOF) {
		return ret;
	}
	putc('\n', fp);

	return ret;
}

FILE* orange_file_fopen_rdonly(const char* pathname)
{
	return fopen(pathname, "r");
}

FILE* orange_file_fopen_wronly(const char* pathname)
{
	return fopen(pathname, "w");
}

int orange_file_fclose(FILE* fp)
{
	return fclose(fp);
}

int orange_file_create(const char* pathname, mode_t mode)
{
	return creat(pathname, mode);
}

int orange_file_open(const char* pathname, int flags)
{
	return open(pathname, flags);
}

int orange_file_open_rdonly(const char* pathname)
{
	return open(pathname, O_RDONLY);
}

int orange_file_open_wronly(const char* pathname)
{
	return open(pathname, O_WRONLY | O_CREAT, 0600);
}

int orange_file_close(int fd)
{
	return close(fd);
}

int orange_file_read(int fd, void* buf, unsigned int size)
{
	int ret = EINVAL;

	ret = read(fd, buf, size);

	if (ret == size) {
		ret = 0;
	} else {
		if (ret == 0) {
			ret = EINVAL;
		} else {
			ret = errno;
		}
	}

	return ret;
}

int orange_file_write(int fd, void* buf, unsigned int size)
{
	int ret = EINVAL;

	ret = write(fd, buf, size);

	if (ret == size) {
		ret = 0;
	} else {
		if (ret == 0) {
			ret = EINVAL;
		} else {
			ret = errno;
		}
	}

	return ret;
}

int orange_file_get_size(int fd, unsigned int* size)
{
	int			ret = EINVAL;
	struct stat sb;

	if (fd < 0 || size == NULL) {
		goto exit;
	}

	memset(&sb, 0, sizeof(struct stat));

	ret = fstat(fd, &sb);
	if (ret != 0) {
		goto exit;
	}
	*size = sb.st_size;

exit:

	return ret;
}

int orange_file_read_line(int fd, void* buf, unsigned int size)
{
	char  c;
	int   length	= 0;
	int   read_size = 0;
	int   i			= 0;
	char* tmp		= (char*) buf;

	if (size > ORANGE_FILE_LINE_MAX) {
		size = ORANGE_FILE_LINE_MAX;
	}

	read_size = read(fd, tmp, size);
	if (read_size <= 0) {
		return -1;
	}

	for (i = 0; i < read_size; i++) {
		if (tmp[i] == 0x0a || tmp[i] == 0x0d) {
			break;
		}
		length++;
	}

	if (length == read_size) {
		do {
			i = read(fd, &c, 1);
		} while (i > 0 && c != 0x0a && c != 0x0d);
		tmp[length] = '\0';
		return length;
	}
	tmp[length] = '\0';

	lseek(fd, length - read_size + 1, SEEK_CUR);

	return length;
}

int orange_file_write_line(int fd, void* buf, unsigned int size)
{
	int ret = -1;

	if (size > ORANGE_FILE_LINE_MAX) {
		/* size must less than ORANGE_FILE_LINE_MAX */
		return ret;
	}

	ret = orange_file_write(fd, buf, size);
	if (ret != 0) {
		return ret;
	}

	unsigned char c = 0x0a;

	ret = orange_file_write(fd, &c, 1);

	return ret;
}

int orange_file_remove(const char* pathname)
{
	return unlink(pathname);
}
