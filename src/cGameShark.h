#ifndef BIOGB_GAMESHARK
#define BIOGB_GAMESHARK

struct GS_Code {
	bool active;
	unsigned char bank;
	unsigned char value;
	unsigned short address;
};

class cGameShark {
private:
	int numOfCodes;
	GS_Code *codes;
	
	cMemory *memPtr;

public:
	cGameShark();
	~cGameShark();
	
	bool loadGSFile(char *fileName);
	void getMemoryPointer(cMemory *mem);
	void turnGSOn(void);
	void turnGSOff(void);
	void writeCodes(void);
};

#endif
