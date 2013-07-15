#include <string.h>		// strlen
#include <stdio.h>		// fopen, fclose
#include <sys/types.h>	// open
#include <sys/stat.h>	// open
#include <fcntl.h>		// open
#include <unistd.h>		// close, read, write
#include <sys/utsname.h>	// uname

/**
 *  @brief open_close_w
 *  @param path_of_file
 *  @return 0 on success, -1 on error
 */
int open_close_w(char *path)
{
	char buf[] = "append by open_close_w()\n";
	int fd = open(path, O_WRONLY|O_APPEND);

	if (fd < 0) {
		fprintf(stderr, "open() error!!\n");
		return -1;
	}
	write(fd, buf, strlen(buf));
	fsync(fd);

	int ret = close(fd);
	if (ret < 0) {
		fprintf(stderr, "close() error!!\n");
		return -1;
	}
	return 0;
}

/**
 *  @brief open_close
 *  @param path_of_file
 *  @return 0 on success, -1 on error
 */
int open_close(char *path)
{
	char buf[1024] = {0};
	int fd = open(path, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "open() error!!\n");
		return -1;
	}
	read(fd, buf, sizeof(buf));
	write(1, buf, strlen(buf));

	int ret = close(fd);
	if (ret < 0) {
		fprintf(stderr, "close() error!!\n");
		return -1;
	}
	return 0;
}

/**
 *  @brief fopen_fclose
 *  @param path_of_file
 *  @return 0 on success, -1 on error
 */
int fopen_fclose(char *path)
{
	char buf[1024] = {0};
	FILE *fp;

	if ((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "fopen() error!!\n");
		return -1;
	}
	while (fgets(buf, 256, fp) != NULL) {
		fputs(buf, stdout);
	}
	fclose(fp);
	return 0;
}

int main(int argc, char ** argv)
{
	const char path[]= "hello.txt";
	char *p;

	if (argc > 1) {
		p = argv[1];
	} else {
		p = path;
	}

	puts("----- 1-1 -----");
	open_close(p);

	puts("----- 1-2 -----");
	fopen_fclose(p);

	puts("----- 1-3 -----");
	open_close_w(p);

	puts("----- 1-4 -----");

	return 0;
}
