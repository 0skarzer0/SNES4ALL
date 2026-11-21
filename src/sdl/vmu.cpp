#include <kos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <unistd.h>
#include "save_icon.h"

#define VMUFILE_PAD 128+512
#define VMUDIR "/vmu/a1/"
#define RAMDIR "/ram/"

static unsigned char    *paquete=NULL;
static int              paquete_size=0;

#ifdef DEBUG_SAVESTATE
#include <unistd.h>
static char ___getcwd[1024];
#define currentdir() getcwd((char *)&___getcwd[0],1023)
#endif

static void eliminate_file(char *filename)
{
#ifdef DEBUG_SAVESTATE
	printf("save: eliminate_file(%s) [%s]\n",filename,currentdir());fflush(stdout);
#endif
	FILE *f=fopen(filename,"r");
	if (f)
	{
		fclose(f);
		unlink(filename);
	}
}

static void prepare_save(void)
{
	if (paquete)
		return;
	char *str="SNES4ALL";
	vmu_pkg_t pkg;
	memset(&pkg, 0, sizeof(pkg));
	strcpy(pkg.desc_short, str);
	strcpy(pkg.desc_long, str);
	strcpy(pkg.app_id, str);
	pkg.icon_cnt = 1;
	pkg.icon_anim_speed = 0;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.eyecatch_data = NULL;
	pkg.data_len = 4;
	pkg.data = (const uint8*)&pkg;
	memcpy((void *)&pkg.icon_pal[0],(void *)&vmu_savestate_icon_pal,32);
	pkg.icon_data = (const uint8*)&vmu_savestate_icon_data;
	vmu_pkg_build(&pkg, &paquete, &paquete_size);
	paquete_size-=4;
}

static void rebuild_paquete(unsigned size, void* _data, FILE *f)
{
	extern char  CMemory_ROMName [];
	char  *name=(char *)CMemory_ROMName;
	unsigned  char *data=(unsigned char *)_data;
	unsigned short *crc=(unsigned short*) &paquete[0x46];
	unsigned *data_len=(unsigned *) &paquete[0x48];
	char *desc_long=(char *) &paquete[16];
	bzero(desc_long,32);
	strncpy(desc_long,name,31);
	*data_len=size;
	int i, c, n = 0;
	(*crc)=0;
	for (i = 0; i < paquete_size; i++)
	{
		if (i<VMUFILE_PAD)
			n ^= (paquete[i]<<8);
		else
			n ^= (data[i-VMUFILE_PAD]<<8);
		for (c = 0; c < 8; c++)
			if (n & 0x8000)
				n = (n << 1) ^ 4129;
			else
				n = (n << 1);
	}
	(*crc)=(n & 0xffff);
	fwrite((void *)&paquete[0],1,VMUFILE_PAD,f);
}



static unsigned getFreeBlocks(void)
{
	int free_blocks=0;
	maple_device_t *dev;

#ifdef DEBUG_SAVESTATE
	puts("save: getFreeBlocks");fflush(stdout);
#endif
	uint8 v=maple_first_vmu();
	if (!v)
		return 0;
	if (maple_compat_resolve(v,&dev,MAPLE_FUNC_MEMCARD)!=0)
		return 0;
#ifdef DEBUG_SAVESTATE
	puts("save: getFreeBlocks - mapped");fflush(stdout);
#endif
	free_blocks=vmufs_free_blocks(dev);
#ifdef DEBUG_SAVESTATE
	printf("save: getFreeBlocks - return %i\n",free_blocks);fflush(stdout);
#endif
	return free_blocks;
}

#ifdef DEBUG_SAVESTATE
static unsigned calculachk(void  *_data, int  len) {
	unsigned *data=(unsigned *)_data;
	unsigned ret=0;
	for(int i=0;i<(len/4);i++) {
		ret+=(i+1)*data[i];
	}
	return ret;
}
#endif

int save_to_vmu(char *fname)
{
#ifdef DEBUG_SAVESTATE
	printf("save_to_vmu '%s'\n",fname);fflush(stdout);
#endif
	char filename[1024];
	int ret=1;
	strcpy(filename,RAMDIR);
	strcat(filename,fname);
	FILE *fi=fopen(filename,"rb");
#ifdef DEBUG_SAVESTATE
	printf("save: abrimos %s\n",filename);
#endif
	if (fi) {
		ret=2;
		fseek(fi,0,SEEK_END);
		long len=ftell(fi);
#ifdef DEBUG_SAVESTATE
		printf("save: la longitud es %i\n",len);
#endif
		if (len>0) {
			void *buf=malloc(len);
			strcpy(filename,VMUDIR);
			strcat(filename,fname);
			FILE *fo=fopen(filename,"rb");
#ifdef DEBUG_SAVESTATE
			printf("save: abrimos %s\n",filename);
#endif
			unsigned libres=getFreeBlocks();
			fseek(fi,0,SEEK_SET);
			int  quitalo=fread(buf,1,len,fi);
#ifdef DEBUG_SAVESTATE
			printf("save: leemos %i de origen (%.8X)\n",quitalo,calculachk(buf,len));
#endif
			prepare_save();
			ret=3;
			if (fo) {
				fseek(fi,0,SEEK_END);
				long l=ftell(fi);
#ifdef DEBUG_SAVESTATE
				printf("save: existe len=%i\n",l);
#endif
				if  (l%512) l+=512;
				libres+=l/512;
				fclose(fo);
			}
#ifdef DEBUG_SAVESTATE
			else
				puts("save: no existe");
#endif
			if ((libres*512)>=(len+512+VMUFILE_PAD)){
				ret=3;
				fo=fopen(filename,"wb");
#ifdef DEBUG_SAVESTATE
				printf("save: abrimos escritura %s\n",filename);
#endif
				if (fo) {
					ret=0;
					rebuild_paquete(len, buf, fo);
					fwrite((void *)&len,1,4,fo);
					int quitar=fwrite(buf,1,len,fo);
					fclose(fo);
#ifdef DEBUG_SAVESTATE
					printf("save: escribimos %i + 4 + %i = %i (%.8X)\n",quitar,VMUFILE_PAD, VMUFILE_PAD+4+quitar,calculachk(buf,quitar));
#endif
				}
			}
			free(buf);
		}
		fclose(fi);
	}
#ifdef DEBUG_SAVESTATE
	printf("save_to_vmu - return %i\n",ret);fflush(stdout);
#endif
	return ret;
}

int load_from_vmu(char *fname)
{
#ifdef DEBUG_SAVESTATE
	printf("load_from_vmu '%s'\n",fname);fflush(stdout);
#endif
	int ret=1;
	char buf[1024];
	strcpy(buf,RAMDIR);
	strcat(buf,fname);
	FILE *fi=fopen(buf,"rb");
	if (fi) {
		ret= -1;
		fclose(fi);
	} else {
	strcpy(buf,VMUDIR);
	strcat(buf,fname);
	fi=fopen(buf,"rb");
#ifdef DEBUG_SAVESTATE
	printf("save: abrimos %s\n",buf);
#endif
	if (fi) {
		ret=2;
		fseek(fi,0,SEEK_END);
		long len=ftell(fi);
#ifdef DEBUG_SAVESTATE
		printf("save: la longitud es %i\n",len);
#endif
		if (len>VMUFILE_PAD) {
			ret=3;
			strcpy(buf,RAMDIR);
			strcat(buf,fname);
			FILE *fo=fopen(buf,"wb");
#ifdef DEBUG_SAVESTATE
			printf("save: abrimos %s\n",buf);
#endif
			if (fo) {
				fseek(fi,VMUFILE_PAD,SEEK_SET);
				fread((void *)&len,1,4,fi);
#ifdef DEBUG_SAVESTATE
				printf("save: Len real %i\n",len);
#endif
				int procesados=0;
				for(;len>0;len-=1024) 
					procesados+=fwrite((void *)buf,1,fread((void *)buf,1,1024,fi),fo);
				ret=0;
				fclose(fo);
#ifdef DEBUG_SAVESTATE
				void *b=malloc(procesados);
				strcpy(buf,RAMDIR);
				strcat(buf,fname);
				fo=fopen(buf,"rb");
				fread(b,1,procesados,fo);
				fclose(fo);
				printf("save: Proceados %i (%.8X)\n",procesados,calculachk(b,procesados));
				free(b);
#endif
			}
		}
		fclose(fi);
	}
	}
#ifdef DEBUG_SAVESTATE
	printf("load_from_vmu - return %i\n",ret);fflush(stdout);
#endif
	return ret;
}
