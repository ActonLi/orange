#include <opt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0
#define DEBUGP printf
#else
#define DEBUGP(args, ...)
#endif

typedef struct config_info {
	int   sw_id;
	char* key;
	char* sign;
	char* xml;
	char* image;
	char* sn;
	char* model;
} config_info_t;

static struct config_info config;

static void __usage(void)
{
	fprintf(stderr, "package -h; to see all options\n");
	return;
}

static int __package_parse(int* argc, char*** argv)
{
	char buff[1024];

	int   para_nums = *argc;
	char* key;
	char* sign;
	char* xml;
	;
	char* image;
	char* sn;
	char* model;
	int   usage = 0;
	int   sw	= 0;

	memset(&config, 0, sizeof(struct config_info));

	snprintf(buff, 1023, "ABB: Package\n\n");

	optTitle(buff);

	optProgName("ABB");

	optrega(&key, OPT_STRING, 'k', "key", " private key");
	optrega(&sign, OPT_STRING, 's', "sign", " sign filename");
	optrega(&xml, OPT_STRING, 'x', "xml", " xml filename");
	optrega(&sn, OPT_STRING, 'n', "serial", " serial number");
	optrega(&image, OPT_STRING, 'i', "image", " image filename");
	optrega(&sw, OPT_INT, 't', "type", " software id");
	optrega(&model, OPT_STRING, 'm', "model", " xml model filename");
	optrega(&usage, OPT_FLAG, 'h', "help", " display usage info");

	opt(argc, argv);

	if (usage || para_nums <= 1) {
		__usage();
		optPrintUsage();
		return 0;
	}

	if (optinvoked(&sw)) {
		config.sw_id = sw;
	}

	if (optinvoked(&key)) {
		config.key = key;
	}

	if (optinvoked(&sign)) {
		config.sign = sign;
	}

	if (optinvoked(&xml)) {
		config.xml = xml;
	}

	if (optinvoked(&image)) {
		config.image = image;
	}

	if (optinvoked(&sn)) {
		config.sn = sn;
	}

	if (optinvoked(&model)) {
		config.model = model;
	}

	return 0;
}

static int __package_get_file_md5(char* filename, char* md5)
{
	FILE* fp		= NULL;
	char  cmd[1024] = {0};
	int   size		= -1;
	int   i;
	int   ret = -1;

	snprintf(cmd, 1024, "md5sum %s", filename);

	fp = popen(cmd, "r");
	if (fp) {
		ret = fread(cmd, 1024, 1, fp);
		if (ret) {
		}
		for (i = 0; i < 1024 && cmd[i] != ' '; i++) {
			md5[i] = cmd[i];
		}

		printf("filename: %s, md5: %s\n", filename, md5);
		pclose(fp);
	}
	return size;
}

static int __package_get_file_size(char* filename)
{
	FILE* fp		= NULL;
	char  cmd[1024] = {0};
	int   size		= -1;
	int   ret		= -1;

	snprintf(cmd, 1024, "stat %s | grep Size", filename);

	fp = popen(cmd, "r");
	if (fp) {
		ret = fread(cmd, 1024, 1, fp);
		if (ret) {
		}
		char* size_p = strstr(cmd, "Size:");
		if (size_p) {
			size_p += strlen("Size:");
			if (*size_p == ' ') {
				size_p++;
			}
			size = atoi(size_p);
		}
		pclose(fp);
	}
	return size;
}

static char buf[8192];
static char new_buf[8192];
static int  __package_create_xml(void)
{
	int   ret		= -1;
	FILE* fp		= NULL;
	int   file_size = 0;
	char  md5[1024] = {0};
	char* filename;

	fp = fopen(config.model, "r");
	if (NULL == fp) {
		goto exit;
	}

	ret = fread(buf, 1, 8192, fp);
	if (ret) {
	}
	fclose(fp);

	file_size = __package_get_file_size(config.image);
	__package_get_file_md5(config.image, md5);

	filename = strrchr(config.image, '/');
	if (filename != NULL) {
		filename++;
	} else {
		filename = config.image;
	}

	printf("filesize: %d, filename: %s, md5: %s\n", file_size, filename, md5);

	printf("buf: %s\n", buf);

	snprintf(new_buf, 8192, buf, filename, file_size, md5, config.sn, config.sw_id);

	printf("newbuf: %s\n", new_buf);

	fp = fopen(config.xml, "w+");

	fwrite(new_buf, 1, strlen(new_buf), fp);
	fclose(fp);

	ret = 0;
exit:
	return ret;
}

static int __package_get_system_return_value(int status)
{
	int ret = -1;

	if (-1 == status) {
		DEBUGP("system error\n");
	} else {
		if (WIFEXITED(status)) {
			if (0 == WEXITSTATUS(status)) {
				DEBUGP("run successfully\n");
				ret = 0;
			} else {
				DEBUGP("run failed\n");
			}
		} else {
			DEBUGP("exit code\n");
		}
	}

	return ret;
}

static int __package_sign_img(void)
{
	int  ret = 0;
	char cmd[1024];
	char sign[64] = "";

	snprintf(sign, 64, "%s.sign", config.image);

	snprintf(cmd, 1024, "openssl dgst -sign %s -sha1 -out %s %s", config.key, sign, config.image);
	ret = system(cmd);

	ret = __package_get_system_return_value(ret);

	return ret;
}

static int __package_sign_xml(void)
{
	int  ret = 0;
	char cmd[1024];
	char sign[64] = "";

	snprintf(sign, 64, "%s.sign", config.xml);

	snprintf(cmd, 1024, "openssl dgst -sign %s -sha1 -out %s %s", config.key, sign, config.xml);
	ret = system(cmd);

	ret = __package_get_system_return_value(ret);

	return ret;
}

static int __package_zip_files(void)
{
	int  ret = 0;
	char cmd[1024];

	snprintf(cmd, 1024, "zip firmware.zip *.sign %s %s", config.xml, config.image);
	printf("%s:%d cmd: %s\n", __func__, __LINE__, cmd);
	ret = system(cmd);

	ret = __package_get_system_return_value(ret);

	return ret;
}

int main(int argc, char** argv)
{
	int ret = -1;

	if (argc < 2) {
		__usage();
		goto exit;
	}

	ret = __package_parse(&argc, &argv);
	if (ret != 0) {
		__usage();
		goto exit;
	}

	ret = __package_create_xml();
	if (ret) {
		goto exit;
	}

	ret = __package_sign_xml();
	if (ret) {
		goto exit;
	}

	ret = __package_sign_img();
	if (ret) {
		goto exit;
	}

	ret = __package_zip_files();

exit:
	return ret;
}
