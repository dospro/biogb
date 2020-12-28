#ifndef BIOGB_CCONFIG
#define BIOGB_CCONFIG

struct sConfig {


	//Graphics config;
	uint xRes, yRes;
	u8 bpp;
	bool filter;
	
	//Sound Config;
	uint soundFreq;
	uint soundBufferSize;
	u8 soundBps;
	
	//Network
	char *netFileName;
	
	//Gameshark
	char *gsFileName;
};

sConfig readConfigFile(char *fileName);

#endif
