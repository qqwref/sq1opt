#include <fstream>
#include <iostream>
#include <cstring>
#include <ctime>

#define NUMHALVES 13
#define NUMLAYERS 158
#define NUMSHAPES 7356
#define FILESTT "sq1stt.dat"
#define FILESCTE "sq1scte.dat"
#define FILESCTC "sq1sctc.dat"
#define FILEP1U  "sq1p1u.dat"
#define FILEP2U  "sq1p2u.dat"
#define FILEP1W  "sq1p1w.dat"
#define FILEP2W  "sq1p2w.dat"

const char* errors[]={
	"Unrecognised command line switch.", //1
	"Too many command line arguments.",
	"Input file not found.",//3
	"Bracket ) expected.",//4
	"Bottom layer turn expected.",//5
	"Comma expected.",//6
	"Top layer turn expected.",//7
	"Bracket ( expected.",//"8
	"Position should be 16 or 17 characters.",//9
	"Expected A-H or 1-8.",//10
	"Expected - or /.",//11
	"Twist is blocked by corner.",//12
	"Can't parse input as position string or movelist.",//13
	"Unexpected bracket (.",//14
	"Number expected.",//15
	"Twist / expected.",//16
	"Position string has too many copies of a piece.",//17
	"Can't stay in cube shape and also use 2gen.",//18
	"Position can't be solved with these constraints",//19
};

const int KARNOTATION_LEN = 109;
const std::string KARNOTATION[KARNOTATION_LEN][2]={
	{"U", "3,0"},
	{"U'", "9,0"},
	{"U2", "6,0"},
	{"D", "0,3"},
	{"D'", "0,9"},
	{"D2", "0,6"},
	{"u", "2,&"},
	{"u'", "^,1"},
	{"d", "&,2"},
	{"d'", "1,^"},
	{"E", "3,9"},
	{"E'", "9,3"},
	{"e", "3,3"},
	{"e'", "9,9"},
	{"F", "4,1"},
	{"F'", "8,&"},
	{"f", "1,4"},
	{"f'", "&,8"},
	{"M", "1,1"},
	{"M'", "&,&"},
	{"m", "2,2"},
	{"m'", "^,^"},
	{"u2", "5,&"},
	{"u2'", "7,1"},
	{"d2", "&,5"},
	{"d2'", "1,7"},
	{"T", "2,8"},
	{"T'", "^,4"},
	{"t", "4,^"},
	{"t'", "8,2"},
	{"W ", "3,0/9,0/"},
	{"W' ", "9,0/3,0/"},
	{"B ", "0,3/0,9/"},
	{"B' ", "0,9/0,3/"},
	{"w ", "2,&/^,1/"},
	{"w' ", "^,1/2,&/"},
	{"b ", "&,2/1,^/"},
	{"b' ", "1,^/&,2/"},
	{"E\\ ", "3,0/0,9/"},
	{"E\\' ", "9,0/0,3/"},
	{"e\\ ", "3,0/0,3/"},
	{"e\\' ", "9,0/0,9/"},
	{"F2 ", "4,1/8,&/"},
	{"F2' ", "8,&/4,1/"},
	{"f2 ", "1,4/&,8/"},
	{"f2' ", "&,8/1,4/"},
	{"U3 ", "3,0/9,0/3,0/"},
	{"U3' ", "9,0/3,0/9,0/"},
	{"D3 ", "0,3/0,9/0,3/"},
	{"D3' ", "0,9/0,3/0,9/"},
	{"u3 ", "2,&/^,1/2,&/"},
	{"u3' ", "^,1/2,&/^,1/"},
	{"u4 ", "2,&/^,1/2,&/^,1/"},
	{"u4' ", "^,1/2,&/^,1/2,&/"},
	{"d3 ", "&,2/1,^/&,2/"},
	{"d3' ", "1,^/&,2/1,^/"},
	{"d4", "&,2/1,^/&,2/&,2"},
	{"d4' ", "1,^/&,2/1,^/&,2/"},
	{"UU", "1,0/5,&/9,0/1,1/9,0/&,0"},
	{"UU'", "1,0/2,&/1,1/2,&/7,1/&,0"},
	{"FV", "0,&/1,^/&,2/1,^/&,2/0,1"},
	{"VF", "1,0/2,&/^,1/2,&/^,1/&,0"},
	{" JJ ", "/0,9/3,3/9,0/"},
	{" jJ ", "/0,9/3,3/9,0/"},
	{" Jj ", "/0,9/3,3/9,0/"},
	{" jj ", "/0,9/3,3/9,0/"},
	{" bJJ ", "/9,0/3,3/0,9/"},
	{" bjJ ", "/9,0/3,3/0,9/"},
	{" bJj ", "/9,0/3,3/0,9/"},
	{" bjj ", "/9,0/3,3/0,9/"},
	{" JN ", "/0,9/0,3/0,9/0,3/"},
	{" jN ", "/0,9/0,3/0,9/0,3/"},
	{" Jn ", "/0,9/0,3/0,9/0,3/"},
	{" jn ", "/0,9/0,3/0,9/0,3/"},
	{" NN ", "/9,3/3,9/"},
	{" Nn ", "/9,3/3,9/"},
	{" nN ", "/9,3/3,9/"},
	{" nn ", "/9,3/3,9/"},
	{" NJ ", "/3,0/9,0/3,0/9,0/"},
	{" nJ ", "/3,0/9,0/3,0/9,0/"},
	{" Nj ", "/3,0/9,0/3,0/9,0/"},
	{" nj ", "/3,0/9,0/3,0/9,0/"},
	{" 3Adj ", "/3,0/&,&/^,1/"},
	{" 03Adj ", "/0,3/&,&/1,^/"},
	{" JR ", "/9,9/2,&/^,1/3,3/"},
	{" jR ", "/9,9/2,&/^,1/3,3/"},
	{" Jr ", "/9,9/1,^/&,2/3,3/"},
	{" jr ", "/9,9/1,^/&,2/3,3/"},
	{" RJ ", "/3,3/1,^/&,2/9,9/"},
	{" rJ ", "/3,3/2,&/^,1/9,9/"},
	{" Rj ", "/3,3/1,^/&,2/9,9/"},
	{" rj ", "/3,3/2,&/^,1/9,9/"},
	{" bRJ ", "/9,9/^,1/2,&/3,3/"},
	{"brJ ", "1,0/9,9/&,2/1,^/3,3/&,0/"},
	{"bRj ", "0,&/9,9/^,1/2,&/3,3/0,1/"},
	{"brj ", "1,&/9,9/&,2/1,^/3,3/&,1/"},
	{"RR ", "1,0/2,&/^,4/5,&/^,1/&,0/"},
	{"rr ", "0,&/^,1/5,&/^,4/2,&/0,1/"},
	{"pJ", "0&/^,1/2,2/0,9/0,1"},
	{"pj", "0,&/1,^/2,2/9,0/0,1"},
	{"pN", "1,0/2,8/^,4/&,0"},
	{"fpJ", "1,0/2,&/^,^/0,3/&,0"},
	{"AA ", "1,0/0,9/2,2/0,9/^,4/&,0/"},
	{"aa", "0&/1,^/2,2/1,^/8,2/0,1"},
	{"TT", "1,0/5,&/9,0/^,^/0,3/&,0"},	
	{"OppOpp", "1,0/&,&/6,0/1,1/&,0"},
	{"FF", "1,0/0,9/2,2/0,9/1,1/9,3/&,0"},
	{"M2", "1,0/&,&/0,1"},
	{"m2", "1,0/5,&/7,1/&,0"}
};

int verbosity = 5;
bool generator=false;
bool usenegative=false;
bool usebrackets=false;
bool karnotation=false;


class HalfLayer {
public:
	int pieces, turn, nPieces;
	HalfLayer(int p, int t) {
		int nEdges=0;
		pieces = p;
		for(int i=0, m=1; i<6; i++, m<<=1){
			if( (pieces&m)!=0 ) nEdges++;
		}
		nPieces=3+nEdges/2;
		turn=t;
	}
};

class Layer {
public:
	HalfLayer& h1, & h2;
	int turnt, turnb;
	int nPieces;
	bool turnParityOdd;
	bool turnParityOddb;
	int pieces;
	int tpieces, bpieces;   // result after turn

	Layer( HalfLayer& p1, HalfLayer& p2): h1(p1), h2(p2) {
		pieces = (h1.pieces<<6)+h2.pieces;
		nPieces = h1.nPieces + h2.nPieces;

		int m=1;
		for(turnt=1; turnt<6; turnt++){
			if( (h1.turn&h2.turn&m)!=0 ) break;
			m<<=1;
		}
		if( turnt==6 ) turnb=6;
		else{
			m=1<<4;
			for(turnb=1; turnb<5; turnb++){
				if( (h1.turn&h2.turn&m)!=0 ) break;
				m>>=1;
			}
		}

		tpieces=pieces;
		int nEdges=0;
		for( int i=0; i<turnt; i++ ){
			if( (tpieces&1)!=0 ) { tpieces+=(1<<12); nEdges++; }
			tpieces>>=1;
		}
		//find out parity of that layer turn
		// Is odd cycle if even # pieces, and odd number passes seam
		//  Note (turn+edges)/2 = number of pieces crossing seam
		turnParityOdd = (nPieces&1)==0 && ((turnt+nEdges)&2)!=0;

		bpieces=pieces;
		nEdges=0;
		for( int i=0; i<turnb; i++ ){
			bpieces<<=1;
			if( (bpieces&(1<<12))!=0 ) { bpieces-=(1<<12)-1; nEdges++; }
		}
		//find out parity of that layer turn
		// Is odd cycle if even # pieces, and odd number passes seam
		//  Note (turn+edges)/2 = number of pieces crossing seam
		turnParityOddb = (nPieces&1)==0 && ((turnb+nEdges)&2)!=0;

	}
};

class Sq1Shape {
public:
	Layer& topl, &botl;
	int pieces;
	bool parityOdd;
	int tpieces[4];
	bool tparity[4];
	Sq1Shape( Layer& l1, Layer& l2, bool p) : topl(l1), botl(l2) {
		parityOdd=p;
		pieces = (l1.pieces<<12)+l2.pieces;
		tpieces[0] = (l1.tpieces<<12)+l2. pieces;
		tpieces[1] = (l1. pieces<<12)+l2.bpieces;
		tpieces[2] = (l1.h1.pieces<<18)+(l2.h1.pieces<<12)+(l1.h2.pieces<<6)+(l2.h2.pieces);
		// calculate mirrored shape
		tpieces[3] = 0;
		for( int m=1, i=0; i<24; i++,m<<=1){
			tpieces[3]<<=1;
			if( (pieces&m)!=0 ) tpieces[3]++;
		}
		tparity[0] = parityOdd^l1.turnParityOdd;
		tparity[1] = parityOdd^l2.turnParityOddb;
		tparity[2] = parityOdd^( (l1.h2.nPieces&1)!=0 && (l2.h1.nPieces&1)!=0 );
		tparity[3] = parityOdd;
	}
};


class ChoiceTable {
public:
	unsigned char choice2Idx[256];
	unsigned char idx2Choice[70];
	ChoiceTable(){
		unsigned char nc=0;
		for( int i=0; i<255; i++ ) choice2Idx[i]=255;
		for( int i=1; i<255; i<<=1 ){
			for( int j=i+i; j<255; j<<=1 ){
				for( int k=j+j; k<255; k<<=1 ){
					for( int l=k+k; l<255; l<<=1 ){
						choice2Idx[i+j+k+l]=nc;
						idx2Choice[nc++]=(unsigned char)(i+j+k+l);
					}
				}
			}
		}
	}
};


class ShapeTranTable {
public:
	int nShape;
	Sq1Shape* shapeList[NUMSHAPES];
	int (*tranTable)[4];
	HalfLayer* hl[NUMHALVES];
	Layer* ll[NUMLAYERS];

	ShapeTranTable(){
		//first build list of possible halflayers
		int hi[]={ 0,    3,12,48, 9,36,33,  15,39,51,57,60,  63};
		int ht[]={42,   43,46,58,45,54,53,  47,55,59,61,62,  63};
		for( int i=0; i<NUMHALVES; i++ ){ hl[i]=new HalfLayer(hi[i],ht[i]); }

		//Now build list of possible Layers
		int lll=0;
		for( int i=0; i<NUMHALVES; i++ ){
			for( int j=0; j<NUMHALVES; j++ ){
				if( hl[i]->nPieces + hl[j]->nPieces<=10 ){
					ll[lll++]=new Layer( *hl[i], *hl[j] );
				}
			}
		}

		//Now build list of all possible shapes
		nShape=0;
		for( int i=0; i<lll; i++ ){
			for( int j=0; j<lll; j++ ){
				if( ll[i]->nPieces + ll[j]->nPieces==16 ){
					shapeList[nShape++]=new Sq1Shape( *ll[i], *ll[j], true );
					shapeList[nShape++]=new Sq1Shape( *ll[i], *ll[j], false );
				}
			}
		}

		// At last we can calculate full transition table
		tranTable = new int[NUMSHAPES][4];
		// see if can be found on file
		std::ifstream is(FILESTT, std::ios::binary);
		if( is.fail() ){
			// no file. calculate table.
			for( int i=0; i<nShape; i++ ){
				//effect on shape of each move, incuding reflection
				for( int m=0; m<4; m++ ){
					for( int j=0; j<nShape; j++ ){
						if( shapeList[i]->tpieces[m] == shapeList[j]->pieces &&
							shapeList[i]->tparity[m] == shapeList[j]->parityOdd ){
							tranTable[i][m]=j;
							break;
						}
					}
				}
			}
			// save to file
			std::ofstream os(FILESTT, std::ios::binary);
			os.write( (char*)tranTable, nShape*4*sizeof(int) );
		}else{
			// read from file
			nShape = NUMSHAPES;
			is.read( (char*)tranTable, nShape*4*sizeof(int) );
		}
	}
	~ShapeTranTable(){
		for( int i=0; i<NUMHALVES; i++ ){ delete hl[i]; }
		for( int i=0; i<NUMLAYERS; i++ ){ delete ll[i]; }
		for( int i=0; i<nShape; i++ ){ delete shapeList[i]; }
		delete[] tranTable;
	}
	inline int getShape(int s, bool p){
		for( int i=0; i<nShape; i++){
			if( shapeList[i]->pieces == s && shapeList[i]->parityOdd==p ) return i;
		}
		return -1;
	}
	inline int getTopTurn(int s){
		return shapeList[s]->topl.turnt;
	}
	inline int getBotTurn(int s){
		return shapeList[s]->botl.turnb;
	}
};

class ShapeColPos {
	ShapeTranTable &stt;
	ChoiceTable &ct;
	int shapeIx;
	int colouring; //24bit string
	bool edgesFlag;
public:
	ShapeColPos( ShapeTranTable& stt0, ChoiceTable& ct0)
		: stt(stt0), ct(ct0) {}
	void set( int shp, int col, bool edges )
	{
		// col is 8 bit colouring of one type of piece.
		// edges set then edge colouring, else corner colouring
		// get full 24 bit colouring.
		int c=ct.idx2Choice[col];
		shapeIx = shp;
		edgesFlag = edges;
		colouring=0;
		int s=stt.shapeList[shapeIx]->pieces;
		if( edges ){
			for( int m=1, i=0, n=1; i<24; m<<=1, i++){
				if( (s&m)!=0 ) {
					if( (c&n)!=0 ) colouring |= m;
					n<<=1;
				}
			}
		}else{
			for( int m=3, i=0, n=1; i<24; m<<=1, i++){
				if( (s&m)==0 ) {
					if( (c&n)!=0 ) colouring |= m;
					n<<=1;
					m<<=1; i++;
				}
			}
		}
	}
	void domove(int m){
		const int botmask = (1<<12)-1;
		const int topmask = (1<<24)-(1<<12);
		const int botrmask = (1<<12)-(1<<6);
		const int toprmask = (1<<18)-(1<<12);
		const int leftmask = botmask+topmask-botrmask-toprmask;
		if( m==0 ){
			int tn=stt.getTopTurn(shapeIx);
			int b=colouring&botmask;
			int t=colouring&topmask;
			t+=(t>>12);
			t<<=(12-tn);
			colouring = b + (t&topmask);
		}else if( m==1 ){
			int tn=stt.getBotTurn(shapeIx);
			int b=colouring&botmask;
			int t=colouring&topmask;
			b+=(b<<12);
			b>>=(12-tn);
			colouring = t + (b&botmask);
		}else if( m==2 ){
			int b=colouring&botrmask;
			int t=colouring&toprmask;
			colouring = (colouring&leftmask) + (t>>6) + (b<<6);
		}
		shapeIx=stt.tranTable[shapeIx][m];
	}
	unsigned char getColIdx(){
		int c=0,n=1;
		int s=stt.shapeList[shapeIx]->pieces;
		if( edgesFlag ){
			for( int m=1, i=0; i<24; m<<=1, i++){
				if( (s&m)!=0 ) {
					if( (colouring&m)!=0 ) c |= n;
					n<<=1;
				}
			}
		}else{
			for( int m=3, i=0; i<24; m<<=1, i++){
				if( (s&m)==0 ) {
					if( (colouring&m)!=0 ) c |= n;
					n<<=1;
					m<<=1; i++;
				}
			}
		}
		return(ct.choice2Idx[c]);
	}
};



class ShpColTranTable {
public:
	char (*tranTable)[70][3];
	ShapeTranTable& stt;
	ChoiceTable& ct;

	ShpColTranTable( ShapeTranTable& stt0, ChoiceTable& ct0, bool edges )
		: stt(stt0), ct(ct0)
	{
		ShapeColPos p(stt,ct);
		tranTable = new char[NUMSHAPES][70][3];

		// see if can be found on file
		std::ifstream is( edges? FILESCTE : FILESCTC, std::ios::binary);
		if( is.fail() ){
			// no file. calculate table.
			// Calculate transition table
			int i,j,m;
			for( m=0; m<3; m++ ){
				for( i=0; i<NUMSHAPES; i++ ){
					for( j=0; j<70; j++){
						p.set(i,j,edges);
						p.domove(m);
						tranTable[i][j][m]=p.getColIdx();
						if( p.getColIdx()==255 ){
							exit(0);
						}
					}
				}
			}
			// save to file
			std::ofstream os(edges? FILESCTE : FILESCTC, std::ios::binary);
			os.write( (char*)tranTable, NUMSHAPES*3*70*sizeof(char) );
		}else{
			// read from file
			is.read( (char*)tranTable, NUMSHAPES*3*70*sizeof(char) );
		}
	}
	~ShpColTranTable(){
		delete[] tranTable;
	}
};

// FullPosition holds position with each piece individually specified.
// Pieces 0-7 are corners and appear twice in a row. Pieces 8-15 are edges and appear once
// Piece numbers below 0 are partially specified corners. Based on the value modulo 3, it's a
//  top corner (0), bottom corner (-2), or any corner (-1).
// Piece numbers above 15 are partially specified edges. Based on the value modulo 3, it's
//  top edge (0), bottom edge (1), or any edge (2).
class FullPosition {
public:
	int pos[24];
	int middle;
	FullPosition(){ reset(); }
	void reset(){
		middle=1;
		for( int i=0; i<24; i++)
			pos[i]="AAIBBJCCKDDLMEENFFOGGPHH"[i]-'A';
	}
	void print(){
		for(int i=0; i<24; i++){
			if (pos[i] < 0) {
				std::cout<<"UWV"[(-pos[i])%3];
			} else if (pos[i] > 15) {
				std::cout<<"XYZ"[pos[i]%3];
			} else {
				std::cout<<"ABCDEFGH12345678"[pos[i]];
			}
			if( pos[i]<8 ) i++;
		}
		std::cout<<"/ -"[middle+1];
	}
	void random(int twoGen, bool keepCubeShape){
		middle = (rand()&1)!=0?-1:1;
		do{
			//make starting position
			int tmp[16];
			for( int i=0; i<8; i++) {
				tmp[2*i + (i>3?1:0)] = i;
				tmp[2*i + (i>3?0:1)] = 8+i;
			}
			// shuffle
			if (keepCubeShape) {
				bool parity = false;
				int cornersToMix = twoGen==1 ? 6 : 8;
				int edgesToMix = twoGen==1 ? 7 : 8;
				for (int i=0; i<cornersToMix; i++) {
					int j = i + rand() % (cornersToMix - i);
					int k = tmp[2*i + (i>3?1:0)];
					tmp[2*i + (i>3?1:0)] = tmp[2*j + (j>3?1:0)];
					tmp[2*j + (j>3?1:0)] = k;
					if (i!=j) parity ^= true;
				}
				for (int i=0; i<edgesToMix; i++) {
					int j = i + rand() % (edgesToMix - i);
					int k = tmp[2*i + (i>3?0:1)];
					tmp[2*i + (i>3?0:1)] = tmp[2*j + (j>3?0:1)];
					tmp[2*j + (j>3?0:1)] = k;
					if (i!=j) parity ^= true;
				}
				if (parity) {
					int k = tmp[0];
					tmp[0] = tmp[2];
					tmp[2] = k;
				}
			} else {
				int nToMix = twoGen==2 ? 12 : (twoGen==1 ? 13 : 16);
				for( int i=0;i<nToMix; i++){
					int j=rand()%(nToMix-i);
					int k=tmp[i];tmp[i]=tmp[i+j];tmp[i+j]=k;
				}
			}
			//convert to position array
			for(int i=0, j=0;i<16;i++){
				pos[j++]=tmp[i];
				if( tmp[i]<8 ) pos[j++]=tmp[i];
			}
			// if p2g and keeping cubeshape, are the corners solvable in 2gen? if not, try again
			if (twoGen == 1 && keepCubeShape) {
				if (!has2GenCorners()) {
					pos[6] = pos[5]; continue; // fail the condition
				}
			}
			// ABF. if keeping cube shape, adjust both; otherwise, if p2g, adjust D only
			if (keepCubeShape) {
				if ((rand()&1)!=0) {
					tmp[0] = pos[11];
					for (int i=10; i>=0; i--) {
						pos[i+1] = pos[i];
					}
					pos[0] = tmp[0];
				}
				if ((rand()&1)!=0) {
					tmp[0] = pos[12];
					for (int i=12; i<=22; i++) {
						pos[i] = pos[i+1];
					}
					pos[23] = tmp[0];
				}
			} else if (twoGen == 1 && (rand()&1)!=0 && pos[11]!=pos[12]) {
				// in pseudo 2gen, with 50% chance, if the layers are validly separated, do a (0,-1)
				tmp[0] = pos[12];
				for (int i=12; i<=22; i++) {
					pos[i] = pos[i+1];
				}
				pos[23] = tmp[0];
			}
			// test twistable
		}while( pos[5]==pos[6] || pos[11]==pos[12] || pos[17]==pos[18] || pos[12]==pos[23]);
	}
	void set(int p[],int m){
		for(int i=0;i<24;i++)pos[i]=p[i];
		middle=m;
	};
	void doTop(int m){
		m%=12;
		if(m<0)m+=12;
		while(m>0){
			int c=pos[11];
			for(int i=11;i>0;i--) pos[i]=pos[i-1];
			pos[0]=c;
			m--;
		}
	}
	void doBot(int m){
		m%=12;
		if(m<0)m+=12;
		while(m>0){
			int c=pos[23];
			for(int i=23;i>12;i--) pos[i]=pos[i-1];
			pos[12]=c;
			m--;
		}
	}
	bool doTwist(){
		if( !isTwistable() ) return false;
		for(int i=6;i<12;i++){
			int c=pos[i];
			pos[i]=pos[i+6];
			pos[i+6]=c;
		}
		middle=-middle;
		return true;
	}
	bool isTwistable(){
		return( pos[0]!=pos[11] && pos[5]!=pos[6] && pos[12]!=pos[23] && pos[17]!=pos[18] );
	}
	int getShape(){
		int s=0;
		for(int m=1<<23,i=0; i<24; i++,m>>=1){
			if(pos[i]>=8) s|=m;
		}
		return(s);
	}
	bool getParityOdd(){
		bool p=false;
		for(int i=0; i<24; i++){
			for(int j=i; j<24; j++){
				if( pos[j]<pos[i]) p=!p;
				if(pos[j]<8)j++;
			}
			if(pos[i]<8)i++;
		}
		return(p);
	}
	int getEdgeColouring(int cl){
		const int clp[3][4]={ { 8, 9,10,11}, { 8, 9,13,14}, {15,14,10, 9} };
		int c=0;
		int cnt=0;
		int m=(cl!=2)?1<<7:1;
		for(int i=0; i<24; i++){
			if( pos[i]>=8 ){
				for(int j=0; j<4; j++){
					if( pos[i]==clp[cl][j] || (pos[i]>15 && pos[i]%3==0 && cl==0)) { // edge up
						c|=m;
						cnt++;
						break;
					}
				}
				if(cl!=2) m>>=1; else m<<=1;
			}
		}
		if (cnt==4) return c;
		else return -1;
	}
	int getCornerColouring(int cl){
		const int clp[3][4]={ {0,1,2,3}, {0,1,5,6}, {7,6,2,1} };
		int c=0;
		int cnt=0;
		int m=(cl!=2)?1<<7:1;
		for(int i=0; i<24; i++){
			if( pos[i]<8 ){
				for(int j=0; j<4; j++){
					if( pos[i]==clp[cl][j] || (pos[i]<0 && pos[i]%3==0 && cl==0)) { // corner up
						c|=m;
						cnt++;
						break;
					}
				}
				if(cl!=2) m>>=1; else m<<=1;
				i++;
			}
		}
		if (cnt==4) return c;
		else return -1;
	}
	bool parseNumberForward(const char*inp, int& ix, int& num){
		bool min = false;
		num = 0;
		while( inp[ix]==' ' ) ix++;
		if( inp[ix]=='-') {
			min=true;
			ix++;
		}
		if( inp[ix]<'0' || inp[ix]>'9' ) return true;
		while( inp[ix]>='0' && inp[ix]<='9' ){
			num =num*10+(inp[ix]-'0');
			ix++;
		}
		if( min ) num = -num;
		while( inp[ix]==' ' ) ix++;
		return false;
	}
	bool parseNumberBackward(const char*inp, int& ix, int& num){
		int digvalue = 1;
		num = 0;
		while( ix>=0 && inp[ix]==' ' ) ix--;
		if( ix<0 ) return true;
		if( inp[ix]<'0' || inp[ix]>'9' ) return true;
		while( ix>=0 && inp[ix]>='0' && inp[ix]<='9' ){
			num =num+digvalue*(inp[ix]-'0');
			digvalue*=10;
			ix--;
		}
		if( ix>=0 && inp[ix]=='-'){
			num = -num;
			ix--;
		}
		while( ix>=0 && inp[ix]==' ' ) ix++;
		return false;
	}
	int parseInput( const char* inp ){
		// scan characters
		const char* t=inp;
		int f=0;
		while(*t){
			if( *t == ',' || *t == '(' || *t == ')' || *t == '9' || *t == '0' ){
				f|=1; // cannot be position string, but may be movelist
			}else if( (*t>='a' && *t<='h') || (*t>='A' && *t<='H') || (*t>='u' && *t<='z') || (*t>='U' && *t<='Z') ){
				f|=2; // cannot be movelist, but may be position string
			}else if( *t!='/' && *t!='-' && (*t<'1' || *t>'8') ){
				f|=3; // cannot be either
			}
			t++;
		}
		if( f==3 || f==0 ){
			return(13);
		}

		reset();
		int lw=0,lu=0;
		if( f==1 && !generator){
			// solution move sequence. start parsing from end
			int md=0;
			int i=strlen(inp)-1;
			while( i>=0 ){
				while( i>=0 && inp[i]==' ' ) i--;
				if( md==0 ){   // parsing any move
					if(inp[i]=='/') md = 1;
					else md = 2;
				}else if( md==1 ){
					if(inp[i--]!='/') return 16;
					if(!doTwist()) return 12;
					lu++;lw++;
					md=2;
				}else if( md==2 ){
					int m = 0;
					bool br=false;
					if( inp[i]==')' ) { i--; br=true; }
					// parsing bot turn
					if( parseNumberBackward(inp, i, m) ) return 5;
					m%=12;
					doBot(-m);
					if(m!=0) lu++;
					if( i<0 || inp[i--]!=',' ) return 6;
					// parsing top turn
					if( parseNumberBackward(inp, i, m) ) return 7;
					m%=12;
					doTop(-m);
					if(m!=0) lu++;
					if( br && ( i<0 || inp[i--]!='(' )) return 8;
					md--;
				}
			}
			if( !isTwistable() ) return 12;
			if( verbosity>=2) std::cout<<"Input:"<<inp<<" ["<<lw<<"|"<<lu<<"]"<<std::endl;
		}else if( f==1 ){
			// generating move sequence. start parsing from beginning
			int md=0;
			int i=0;
			while( inp[i]!=0 ){
				while( inp[i]==' ' ) i++;
				if( md==0 ){   // parsing any move
					if(inp[i]=='/') md = 1;
					else md = 2;
				}else if( md==1 ){
					if(inp[i++]!='/') return 16;
					if(!doTwist()) return 12;
					lu++;lw++;
					md=2;
				}else if( md==2 ){
					int m = 0;
					bool br=false;
					if( inp[i]=='(' ) { i++; br=true; }
					// parsing top turn
					if( parseNumberForward(inp, i, m) ) return 7;
					m%=12;
					doTop(m);
					if(m!=0) lu++;
					if( inp[i++]!=',' ) return 6;
					// parsing bot turn
					if( parseNumberForward(inp, i, m) ) return 5;
					m%=12;
					doBot(m);
					if(m!=0) lu++;
					if( br && inp[i++]!=')' ) return 4;
					md--;
				}
			}
			if( !isTwistable() ) return 12;
			if( verbosity>=2) std::cout<<"Input:"<<inp<<" ["<<lw<<"|"<<lu<<"]"<<std::endl;
		}else{
			// position
			if( strlen(inp)!=16 && strlen(inp)!=17 ) return(9);
			int pieceCount[16]; // track counts of each piece, so we can detect multiples of one piece
			int cecount[6]; // track total [up, down, all] + 3*[corners, edges]
			for (int i=0; i<16; i++) pieceCount[i] = 0;
			for (int i=0; i<6; i++) cecount[i] = 0;
			int j=0;
			int pi[24];
			// we can't reuse a piece number because two of the same number means a corner, so
			// each partially defined piece gets a separate set of 3 possible values
			int nextPartialCorner = -3;
			int nextPartialEdge = 18;
			for( int i=0; i<16; i++){
				int k=inp[i];
				if(k>='a' && k<='z') k+=('A'-'a');
				if(k>='A' && k<='H') k-='A';
				else if(k>='1' && k<='8') k-='1'-8;
				else if(k>='U' && k<='W') {
					k+=(nextPartialCorner-'U');
					nextPartialCorner -= 3;
				}
				else if(k>='X' && k<='Z') {
					k+=(nextPartialEdge-'X');
					nextPartialEdge += 3;
				}
				else return(10);
				pi[j++] = k;
				if (k>=0 && k<=15) pieceCount[k]++;
				if (k<8) {
					pi[j++] = k;
					cecount[2]++;
					if ((k<0 && k%3==0) || (k>=0 && k<=3)) cecount[0]++; // corner up
					if ((k<0 && k%3==-2) || (k>=4 && k<=7)) cecount[1]++; // corner down
				} else {
					cecount[5]++;
					if ((k>15 && k%3==0) || (k>=8 && k<=11)) cecount[3]++; // edge up
					if ((k>15 && k%3==1) || (k>=12 && k<=15)) cecount[4]++; // edge down
				}
			}
			for (int i=0; i<16; i++) {
				if (pieceCount[i] > 1) return(17);
			}
			if (cecount[0] > 4 || cecount[1] > 4 || cecount[2] > 8 || cecount[3] > 4 || cecount[4] > 4 || cecount[5] > 8) return 17;
			int midLayer=0;
			if( strlen(inp)==17 ){
				int k=inp[16];
				if( k!='-' && k!='/' ) return(11);
				midLayer = (k=='-') ? 1 : -1;
			}
			set(pi,midLayer);
		}
		return(0);
	}
	// assuming we're in a square/square shape, check if the corners are solvable with 2gen
	bool has2GenCorners(){
		// get corners
		int tmp[6];
		int j=0;
		for (int i=0; i<18; i++) {
			if (pos[i]<8) {
				if (j%2 == 0) tmp[j/2] = pos[i];
				j++;
			}
		}
		// place D corners - if we find a D corner on U, AUF and then insert
		int found_d = -1;
		for (int i=0; i<4; i++) if(tmp[i]>3) found_d = i;
		if (found_d > -1) {
			int tmp2[4];
			for (int i=0; i<4; i++) tmp2[i] = tmp[i];
			for (int i=0; i<4; i++) tmp[i] = tmp2[(i + found_d) % 4];
			int k = tmp[0]; tmp[0] = tmp[4]; tmp[4] = k;
			k = tmp[2]; tmp[2] = tmp[3]; tmp[3] = k;
		}
		found_d = -1;
		for (int i=0; i<4; i++) if(tmp[i]>3) found_d = i;
		if (found_d > -1) {
			int tmp2[4];
			for (int i=0; i<4; i++) tmp2[i] = tmp[i];
			for (int i=0; i<4; i++) tmp[i] = tmp2[(i + found_d) % 4];
			int k = tmp[0]; tmp[0] = tmp[5]; tmp[5] = k;
			k = tmp[1]; tmp[1] = tmp[2]; tmp[2] = k;
		}
		// adjust if D corners are swapped, then AUF
		if (tmp[4] == 5 && tmp[5] == 4) {
			tmp[4] = 4; tmp[5] = 5;
			int k = tmp[0]; tmp[0] = tmp[2]; tmp[2] = k;
		}
		int found_u = -1;
		for (int i=0; i<4; i++) if(tmp[i]==0) found_u = i;
		if (found_u > -1) {
			int tmp2[4];
			for (int i=0; i<4; i++) tmp2[i] = tmp[i];
			for (int i=0; i<4; i++) tmp[i] = tmp2[(i + found_u) % 4];
		}
		if (tmp[0] == 0 && tmp[1] == 1 && tmp[2] == 2 && tmp[3] == 3 && tmp[4] == 4 && tmp[5] == 5) return true;
		return false;
	}
	bool singleMatch(int posI, int solvedI) {
		if (posI == solvedI) return true;
		if (posI>15 && posI%3==0  && solvedI >= 8  && solvedI <= 11) return true; // edge up
		if (posI>15 && posI%3==1  && solvedI >= 12 && solvedI <= 15) return true; // edge down
		if (posI<0  && posI%3==0  && solvedI >= 0  && solvedI <= 3)  return true; // corner up
		if (posI<0  && posI%3==-2 && solvedI >= 4  && solvedI <= 7)  return true; // corner down
		if (posI>15 && posI%3==2  && solvedI >= 8  && solvedI <= 15) return true; // edge any
		if (posI<0  && posI%3==-1 && solvedI >= 0  && solvedI <= 7)  return true; // corner any
		return false;
	}
	bool matchesSolved() {
		int solved[24] = {0, 0, 8, 1, 1, 9, 2, 2, 10, 3, 3, 11, 12, 4, 4, 13, 5, 5, 14, 6, 6, 15, 7, 7};
		for (int i=0; i<24; i++) {
			if (!singleMatch(pos[i], solved[i])) return false;
		}
		return true;
	}
	bool isPartial() {
		for (int i=0; i<24; i++) {
			if (pos[i] < 0 || pos[i] > 15) return true;
		}
		return false;
	}
};

//pruning table for combination of shape,edgecolouring,cornercolouring.
class PrunTable {
public:
	char (*table)[70][70];
	ShapeTranTable& stt;
	ShpColTranTable& scte;
	ShpColTranTable& sctc;

	PrunTable( FullPosition& p0, int cl, ShapeTranTable& stt0, ShpColTranTable& scte0, ShpColTranTable& sctc0, bool turnMetric )
		: stt(stt0), scte(scte0), sctc(sctc0)
	{
		// Calculate pruning table
		table = new char[NUMSHAPES][70][70];
		const char *fname;
		if( turnMetric ){
			fname = (cl==0)? FILEP1U : FILEP2U;
		}else{
			fname = (cl==0)? FILEP1W : FILEP2W;
		}

		// see if can be found on file
		std::ifstream is( fname, std::ios::binary );
		if( is.fail() ){
			// no file. calculate table.
			// clear table
			for( int i0=0; i0<NUMSHAPES; i0++ ){
			for( int i1=0; i1<70; i1++){
			for( int i2=0; i2<70; i2++){
				table[i0][i1][i2]=0;
			}}}
			//set start position
			int s0 = stt.getShape(p0.getShape(),p0.getParityOdd());
			int e0 = p0.getEdgeColouring(cl);
			int c0 = p0.getCornerColouring(cl);
			e0 = scte0.ct.choice2Idx[e0];
			c0 = sctc0.ct.choice2Idx[c0];
			if( turnMetric ){
				table[s0][e0][c0]=1;
			}else{
				setAll(s0,e0,c0,1);
			}

			char l=1;
			int n=1;
			do{
				if(verbosity>=6) std::cout<<" l="<<(int)(l-1)<<"  n="<<(int)n<<std::endl;
				n=0;
				if( turnMetric ){
					for( int i0=0; i0<NUMSHAPES; i0++ ){
					for( int i1=0; i1<70; i1++){
					for( int i2=0; i2<70; i2++){
						if( table[i0][i1][i2]==l ){
							for( int m=0; m<3; m++){
								int j0=i0, j1=i1, j2=i2;
								int w=0;
								do{
									j2=sctc.tranTable[j0][j2][m];
									j1=scte.tranTable[j0][j1][m];
									j0=stt.tranTable[j0][m];
									if( table[j0][j1][j2]==0 ){
										table[j0][j1][j2]=l+1;
										n++;
									}
									w++;
									if(w>12){
										exit(0);
									}
								}while(j0!=i0 || j1!=i1 || j2!=i2 );
							}
						}
					}}}
				}else{
					for( int i0=0; i0<NUMSHAPES; i0++ ){
					for( int i1=0; i1<70; i1++){
					for( int i2=0; i2<70; i2++){
						if( table[i0][i1][i2]==l ){
							// do twist
							int j0=stt.tranTable[i0][2];
							int j1=scte.tranTable[i0][i1][2];
							int j2=sctc.tranTable[i0][i2][2];
							if( table[j0][j1][j2]==0 ){
								n+=setAll(j0,j1,j2,l+1);
							}
						}
					}}}
				}
				l++;
			}while(n!=0);
			if(verbosity>=6) std::cout<<std::endl;

		// save to file
			std::ofstream os( fname, std::ios::binary );
			os.write( (char*)table, NUMSHAPES*70*70*sizeof(char) );
		}else{
			// read from file
			is.read( (char*)table, NUMSHAPES*70*70*sizeof(char) );
		}


	}
	~PrunTable(){
		delete[] table;
	}
	// set a position to depth l, as well as all rotations of it.
	inline int setAll(int i0,int i1,int i2, char l){
		int n=0;
		int j0=i0, j1=i1, j2=i2;
		do{
			int k0=j0, k1=j1, k2=j2;
			do{
				if( table[k0][k1][k2]==0 ){
					table[k0][k1][k2]=l;
					n++;
				}
				k2=sctc.tranTable[k0][k2][0];
				k1=scte.tranTable[k0][k1][0];
				k0=stt.tranTable[k0][0];
			}while(j0!=k0 || j1!=k1 || j2!=k2 );
			j2=sctc.tranTable[j0][j2][1];
			j1=scte.tranTable[j0][j1][1];
			j0=stt.tranTable[j0][1];
		}while(j0!=i0 || j1!=i1 || j2!=i2 );
		return n;
	}
};


// PositionSolver holds position encoded by colourings
class PositionSolver {
	public:
	int e0,e1,e2,c0,c1,c2;
	int shp,shp2,middle;
	FullPosition fp;
	ShapeTranTable& stt;
	ShpColTranTable& scte;
	ShpColTranTable& sctc;
	PrunTable& pr1;
	PrunTable& pr2;

	int moveList[50];
	int moveLen;
	int lastTurns[6];
	bool turnMetric;
	bool findAll;
	bool ignoreTrans;

	PositionSolver( ShapeTranTable& stt0, ShpColTranTable& scte0, ShpColTranTable& sctc0, PrunTable& pr10, PrunTable& pr20 )
		: stt(stt0), scte(scte0), sctc(sctc0), pr1(pr10), pr2(pr20) {};
	void set(FullPosition& p, bool turnMetric0, bool findAll0, bool ignoreTrans0){
		int cc0 = p.getCornerColouring(0);
		int cc1 = p.getCornerColouring(1);
		int cc2 = p.getCornerColouring(2);
		c0 = (cc0==-1 ? -1 : sctc.ct.choice2Idx[cc0]);
		c1 = (cc1==-1 ? -1 : sctc.ct.choice2Idx[cc1]);
		c2 = (cc2==-1 ? -1 : sctc.ct.choice2Idx[cc2]);
		int ec0 = p.getEdgeColouring(0);
		int ec1 = p.getEdgeColouring(1);
		int ec2 = p.getEdgeColouring(2);
		e0 = (ec0==-1 ? -1 : scte.ct.choice2Idx[ec0]);
		e1 = (ec1==-1 ? -1 : scte.ct.choice2Idx[ec1]);
		e2 = (ec2==-1 ? -1 : scte.ct.choice2Idx[ec2]);
		shp = stt.getShape(p.getShape(),p.getParityOdd());
		shp2 = stt.tranTable[shp][3];
		middle = p.middle;
		turnMetric=turnMetric0;
		findAll=findAll0;
		ignoreTrans=ignoreTrans0;
		fp = p;
	};
	virtual inline int doMove(int m){
		const int mirrmv[3]={1,0,2};
		int r=0;
		if(m==0){
			r=stt.getTopTurn(shp);
		}else if(m==1){
			r=stt.getBotTurn(shp);
		}else{
			middle=-middle;
		}
		c0 = sctc.tranTable[shp][c0][m];
		c1 = sctc.tranTable[shp][c1][m];
		e0 = scte.tranTable[shp][e0][m];
		e1 = scte.tranTable[shp][e1][m];
		shp = stt.tranTable[shp][m];

		c2 = sctc.tranTable[shp2][c2][mirrmv[m]];
		e2 = scte.tranTable[shp2][e2][mirrmv[m]];
		shp2 = stt.tranTable[shp2][mirrmv[m]];
		return r;
	}
	virtual int solve(int twoGen, int extraMoves, bool keepCubeShape){
		// check that the given position is solvable with these constraints
		if (twoGen == 2) {
			// check that 7G8H are solved
			if (fp.pos[18] != 14 || fp.pos[19] != 6 || fp.pos[20] != 6 || fp.pos[21] != 15 || fp.pos[22] != 7 || fp.pos[23] != 7) return 19;
		} else if (twoGen == 1) {
			// check that G8H are solved or solved-and-ADF
			if (fp.pos[19] == 6 && fp.pos[20] == 6 && fp.pos[21] == 15 && fp.pos[22] == 7 && fp.pos[23] == 7) {
				// ok
			} else if (fp.pos[18] == 6 && fp.pos[19] == 6 && fp.pos[20] == 15 && fp.pos[21] == 7 && fp.pos[22] == 7) {
				// ok
			} else return 19;
		}
		if (keepCubeShape) {
			// check that it's in cube shape and of the right parity
			if (!((shp==5052 || shp==4148 || shp==5039 || shp==4163) && (shp2==5052 || shp2==4148 || shp2==5039 || shp2==4163))) {
				return 19;
			}
			if (twoGen == 1) {
				// check that the corners are 2genable
				if (!fp.has2GenCorners()) return 19;
			}
		}
		
		// run the solve
		moveLen=0;
		unsigned long nodes=0;
		// only even lengths if twist metric and middle is square
		int l=-1;
		if( !turnMetric && middle==1 ) l=-2;
		// do ida
		int optimalMoves = -1;
		while(true){
			l++;
			if( !turnMetric && middle!=0 ) l++;
			if(verbosity>=5) std::cout<<"searching depth "<<l<<std::endl<<std::flush;
			for( int i=0; i<6; i++) lastTurns[i]=0;
			int searchResult = search(l,3, &nodes, twoGen, keepCubeShape);
			if (searchResult != 0) {
				if (optimalMoves == -1) optimalMoves = l;
				if (l >= optimalMoves + extraMoves || (!turnMetric && middle!=0 && l+1 >= optimalMoves + extraMoves)) break;
			}
		};
		return 0;
	}
	virtual inline bool isSolved() {
		if( shp==4163 && e0==69 && e1==44 && e2==44 && c0==69 && c1==44 && c2==44 && middle>=0 ) return true;
		else return false;
	}
	// determine if we should prune this branch of the tree
	virtual inline bool prunedOut(int l) {
		if( pr1.table[shp ][e0][c0]>l+1 ) return true;
		if( pr2.table[shp ][e1][c1]>l+1 ) return true;
		if( pr2.table[shp2][e2][c2]>l+1 ) return true;
		return false;
	}
	int search( const int l, const int lm, unsigned long *nodes, int twoGen, bool keepCubeShape){
		int i,r=0;

		// search for l more moves. previous move was lm.
		(*nodes)++;
		if( l<0 ) return 0;

		//prune based on transformation
		// (a,b)/(c,d)/(e,f) -> (6+a,6+b)/(d,c)/(6+e,6+f)
		// qq note: this step is only done for turn metric, because the pruning steps below are ignored
		if( turnMetric && !ignoreTrans && twoGen == 0){
			// (a,b)/(c,d)/(e,f) -> (6+a,6+b)/(d,c)/(6+e,6+f)
			// moves changes by:
			// a,b,e,f=0/6 -> m++/m--
			i=0;
			if( lastTurns[0]==0 ) i++;
			else if( lastTurns[0]==6 ) i--;
			if( lastTurns[1]==0 ) i++;
			else if( lastTurns[1]==6 ) i--;
			if( lastTurns[4]==0 ) i++;
			else if( lastTurns[4]==6 ) i--;
			if( lastTurns[5]==0 ) i++;
			else if( lastTurns[5]==6 ) i--;
			int absTopMove = lastTurns[0]>6 ? 12-lastTurns[0] : lastTurns[0];
			int absBottomMove = lastTurns[1]>6 ? 12-lastTurns[1] : lastTurns[1];
			if( i<0 || ( i==0 && ((absTopMove + absBottomMove > 6) || (absTopMove + absBottomMove == 6 && absTopMove < absBottomMove)))) return 0;
		}

		// check if it is now solved
		if( l==0 ){
			if(isSolved()){
				printsol();
				if(verbosity>=6) std::cout<<"Nodes="<<*nodes<<std::endl<<std::flush;
				return 1;
			}else if( turnMetric ) return 0;
		}

		// prune
		if(prunedOut(l)) return 0;

		// try all top layer moves
		if( lm>=2 ){
			i=doMove(0);
			do{
				// qq note: Jaap's solver pruned the transformation by only allowing U moves between
				// 0 and 5. I think it's better to do this on D, see below. We then allow any (x,0).
				//if( turnMetric || ignoreTrans || twoGen!=0 || i<6 || l<2 ){
				moveList[moveLen++]=i;
				lastTurns[4]=i;
				r+=search( turnMetric?l-1:l, 0, nodes, twoGen, keepCubeShape);
				moveLen--;
				if(r!=0 && !findAll) return(r);
				//}
				i+=doMove(0);
			}while( i<12);
			lastTurns[4]=0;
		}
		// try all bot layer moves
		if( lm!=1 && twoGen != 2){
			i=doMove(1);
			do{
				// if we're allowed to use the transformation, and we're not doing any kind of
				// 2gen, and we're not in the last two moves, then we should skip this move if the
				// current (x,y) is worse than the alternative.
				// the logic for that is: |x| + |y| >= 7, or |x| + |y| = 6 and |y| > |x|
				int topMove = lastTurns[4];
				int absTopMove = topMove>6 ? 12-topMove : topMove;
				int absBottomMove = i>6 ? 12-i : i;
				if (turnMetric || ignoreTrans || twoGen!=0 || l<2 || (absTopMove + absBottomMove < 6) || (absTopMove + absBottomMove == 6 && absTopMove >= absBottomMove)) {
					moveList[moveLen++]=i+12;
					lastTurns[5]=i;
					if (twoGen != 1 || i==1 || i==11) {
						r+=search( turnMetric?l-1:l, 1, nodes, twoGen, keepCubeShape);
					}
					moveLen--;
					if(r!=0 && !findAll) return(r);
				}
				i+=doMove(1);
			}while( i<12);
			lastTurns[5]=0;
		}
		// try twist move
		if( lm!=2 && l>0){
			int lt0=lastTurns[0], lt1=lastTurns[1];
			lastTurns[0]=lastTurns[2];
			lastTurns[1]=lastTurns[3];
			lastTurns[2]=lastTurns[4];
			lastTurns[3]=lastTurns[5];
			lastTurns[4]=0;
			lastTurns[5]=0;
			doMove(2);
			if (!keepCubeShape || ((shp==5052 || shp==4148 || shp==5039 || shp==4163) && (shp2==5052 || shp2==4148 || shp2==5039 || shp2==4163))) {
				moveList[moveLen++]=0;
				r+=search(l-1, 2, nodes, twoGen, keepCubeShape);
				moveLen--;
				if(r!=0 && !findAll) return(r);
			}
			doMove(2);
			lastTurns[5]=lastTurns[3];
			lastTurns[4]=lastTurns[2];
			lastTurns[3]=lastTurns[1];
			lastTurns[2]=lastTurns[0];
			lastTurns[1]=lt1;
			lastTurns[0]=lt0;
		}
		return r;
	}

	int normaliseMove(int m){
		while(m<0) m+=12;
		while(m>=12) m-=12;
		if( usenegative && m>6 ) m-=12;
		return m;
	}
	std::string printmove(int mu, int md, bool removeAUF){
		std::string out = "";
		if (removeAUF) {
			mu = (mu + 13)%3 - 1;
			md = (md + 13)%3 - 1;
		}
		if( mu!=0 || md!=0 ) {
			if( usebrackets && !karnotation ) out += "(";
			out += std::to_string(mu) + "," + std::to_string(md);
			if( usebrackets && !karnotation ) out += ")";
		}
		return out;
	}
	std::string replaceAll(std::string str, std::string olds, std::string news) {
		size_t pos;
		while ((pos = str.find(olds)) != std::string::npos) {
			str.replace(pos, olds.length(), news);
		}
		return str;
	}
	void printsol(){
		std::string out = "";
		int tw=0, tu=0;
		int mu=0, md=0;
		if( generator ){
			for( int i=moveLen-1; i>=0; i--){
				if( moveList[i]==0 ){
					out += printmove(mu, md, (tw==0 && karnotation));
					mu = md = 0;
					out += "/";
					tu++; tw++;
				}else if( moveList[i]<12 ){
					mu = normaliseMove(mu-moveList[i]);
					tu++;
				}else{
					md = normaliseMove(md+moveList[i]);
					tu++;
				}
			}
		}else{
			for( int i=0; i<moveLen; i++){
				if( moveList[i]==0 ){
					out += printmove(mu, md, (tw==0 && karnotation));
					mu = md = 0;
					out += "/";
					tu++; tw++;
				}else if( moveList[i]<12 ){
					mu = normaliseMove(mu+moveList[i]);
					tu++;
				}else{
					md = normaliseMove(md-moveList[i]);
					tu++;
				}
			}
		}
		out += printmove(mu, md, karnotation);
		if (karnotation) {
			out = replaceAll(out, std::string(" "), std::string(""));
			
			// replace all negative numbers to avoid e.g. (-2,-4) -> -T
			out = replaceAll(out, std::string("-1"), std::string("&"));
			out = replaceAll(out, std::string("-2"), std::string("^"));
			out = replaceAll(out, std::string("-3"), std::string("9"));
			out = replaceAll(out, std::string("-4"), std::string("8"));
			out = replaceAll(out, std::string("-5"), std::string("7"));
			
			// do the karnotation replacements
			for (int i = KARNOTATION_LEN-1; i>=0; i--)
			{
				out = replaceAll(out, KARNOTATION[i][1], KARNOTATION[i][0]);
			}
			
			out = replaceAll(out, std::string("/"), std::string(" "));
			out = replaceAll(out, std::string("\\"), std::string("/")); // handle slashes for E/, e/
			out = replaceAll(out, std::string(","), std::string(""));
			// undo number replacement
			out = replaceAll(out, std::string("&"), std::string("-1"));
			out = replaceAll(out, std::string("^"), std::string("-2"));
			out = replaceAll(out, std::string("9"), std::string("-3"));
			out = replaceAll(out, std::string("8"), std::string("-4"));
			out = replaceAll(out, std::string("7"), std::string("-5"));
		}
		std::cout << out;
		std::cout <<"  ["<<tw<<"|"<<tu<<"] "<<std::endl;
	}
};

// PartialositionSolver is like PositionSolver but may have some incompletely defined pieces
class PartialPositionSolver : public PositionSolver {
	int shpx, shpx2; // extra shapes to account for both possible parities

public:
	PartialPositionSolver( ShapeTranTable& stt0, ShpColTranTable& scte0, ShpColTranTable& sctc0, PrunTable& pr10, PrunTable& pr20 )
	    : PositionSolver(stt0, scte0, sctc0, pr10, pr20) {}
	void set(FullPosition& p, bool turnMetric0, bool findAll0, bool ignoreTrans0){
		PositionSolver::set(p, turnMetric0, findAll0, ignoreTrans0);
		shpx = stt.getShape(p.getShape(),!p.getParityOdd());
		shpx2 = stt.tranTable[shpx][3];
	};
	inline int doMove(int m){
		const int mirrmv[3]={1,0,2};
		int r=0;
		if(m==0){
			r=stt.getTopTurn(shp);
			fp.doTop(r);
		}else if(m==1){
			r=stt.getBotTurn(shp);
			fp.doBot(-r);
		}else{
			middle=-middle;
			fp.doTwist();
		}
		// only update c0/c1/e0/e1/c2/e2 if they are not -1
		if (c0>-1) c0 = sctc.tranTable[shp][c0][m];
		if (c1>-1) c1 = sctc.tranTable[shp][c1][m];
		if (e0>-1) e0 = scte.tranTable[shp][e0][m];
		if (e1>-1) e1 = scte.tranTable[shp][e1][m];
		shp = stt.tranTable[shp][m];
		shpx = stt.tranTable[shpx][m];

		if (c2>-1) c2 = sctc.tranTable[shp2][c2][mirrmv[m]];
		if (e2>-1) e2 = scte.tranTable[shp2][e2][mirrmv[m]];
		shp2 = stt.tranTable[shp2][mirrmv[m]];
		shpx2 = stt.tranTable[shpx2][mirrmv[m]];
		return r;
	}
	int solve(int twoGen, int extraMoves, bool keepCubeShape){
		// check that the given position is solvable with these constraints
		if (twoGen == 2) {
			// check that 7G8H are solved
			if (!(fp.singleMatch(fp.pos[18], 14) && fp.singleMatch(fp.pos[19], 6) &&
				fp.singleMatch(fp.pos[20], 6) && fp.singleMatch(fp.pos[21], 15) &&
				fp.singleMatch(fp.pos[22], 7) && fp.singleMatch(fp.pos[23], 7))) return 19;
		} else if (twoGen == 1) {
			// check that G8H are solved or solved-and-ADF
			if (fp.singleMatch(fp.pos[19], 6) && fp.singleMatch(fp.pos[20], 6) &&
				fp.singleMatch(fp.pos[21], 15) && fp.singleMatch(fp.pos[22], 7) &&
				fp.singleMatch(fp.pos[23], 7)) {
				// ok
			} else if (fp.singleMatch(fp.pos[18], 6) && fp.singleMatch(fp.pos[19], 6) &&
				fp.singleMatch(fp.pos[20], 15) && fp.singleMatch(fp.pos[21], 7) &&
				fp.singleMatch(fp.pos[22], 7)) {
				// ok
			} else return 19;
		}
		if (keepCubeShape) {
			// check that it's in cube shape and of the right parity
			if (!((shp==5052 || shp==4148 || shp==5039 || shp==4163) && (shp2==5052 || shp2==4148 || shp2==5039 || shp2==4163))) {
				return 19;
			}
		}
		
		// run the solve
		moveLen=0;
		unsigned long nodes=0;
		// only even lengths if twist metric and middle is square
		int l=-1;
		if( !turnMetric && middle==1 ) l=-2;
		// do ida
		int optimalMoves = -1;
		while(true){
			l++;
			if( !turnMetric && middle!=0 ) l++;
			if(verbosity>=5) std::cout<<"searching depth "<<l<<std::endl<<std::flush;
			for( int i=0; i<6; i++) lastTurns[i]=0;
			int searchResult = search(l,3, &nodes, twoGen, keepCubeShape);
			if (searchResult != 0) {
				if (optimalMoves == -1) optimalMoves = l;
				if (l >= optimalMoves + extraMoves || (!turnMetric && middle!=0 && l+1 >= optimalMoves + extraMoves)) break;
			}
		};
		return 0;
	}
	inline bool isSolved() {
		return (fp.matchesSolved() && middle>=0);
	}
	// determine if we should prune this branch of the tree
	// we should have a shape-only pruning table
	inline bool prunedOut(int l) {
		if (e0>-1 && c0>-1) {
			if( pr1.table[shp ][e0][c0]>l+1 && pr1.table[shpx][e0][c0]>l+1) return true;
		}
		if (e1>-1 && c1>-1) {
			if( pr2.table[shp ][e1][c1]>l+1 && pr2.table[shpx][e1][c1]>l+1) return true;
		}
		if (e2>-1 && c2>-1) {
			if( pr2.table[shp2][e2][c2]>l+1 && pr2.table[shpx2][e2][c2]>l+1) return true;
		}
		return false;
	}
};

int show(int e){
	std::cerr<<errors[e-1]<<std::endl;
	return(e);
}

int parseInteger(const char* s){
	int n=0;
	while( *s!='\0' ){
		if( *s<'0' || *s>'9' ) return -1;
		n = n*10 + (*s -'0');
		s++;
	}
	return n;
}

void help(){
	std::cout<<"Usage:"<<std::endl;
	std::cout<<"  sq1optim <switches> <position>"<<std::endl;
	std::cout<<"  sq1optim <switches> <movesequence>"<<std::endl;
	std::cout<<"  sq1optim <switches>"<<std::endl;
	std::cout<<std::endl;
	std::cout<<"<position> is a string encoding a particular position. For example"<<std::endl;
	std::cout<<"   A1B2C3D45E6F7G8H- is the solved position. Letters represent corners, numbers"<<std::endl;
	std::cout<<"   the edges, starting from the front seam clockwise around the top layer and"<<std::endl;
	std::cout<<"   then clockwise around the bottom layer. Optionally, the middle layer is"<<std::endl;
	std::cout<<"   denoted by a - for a square and / for kite shape."<<std::endl;
	std::cout<<"   You can also partially define pieces:"<<std::endl;
	std::cout<<"   U is a top corner, V is a bottom corner, W is any corner,"<<std::endl;
	std::cout<<"   X is a top edge,   Y is a bottom edge,   Z is any edge."<<std::endl;
	std::cout<<"<movesequence> is a string encoding a sequence of moves. Layer turns are"<<std::endl;
	std::cout<<"   denoted by (t,b) where t and b are integers indicating that the top and"<<std::endl;
	std::cout<<"   bottom layers are turned by t and b twelths of a full circle. Positive"<<std::endl;
	std::cout<<"   numbers are clockwise turns, negative anti-clockwise."<<std::endl;
	std::cout<<"<switches> are one of more of the following command line switches:"<<std::endl;
	std::cout<<"   -w     Use only the number of twists to measure length, not layer turns."<<std::endl;
	std::cout<<"   -a<n>  Generate all optimal sequences, not just the first one found."<<std::endl;
	std::cout<<"          If n is given, also find solutions with up to n extra moves."<<std::endl;
	std::cout<<"   -x     Ignore the equivalence a,b/c,d/e,f = 6+a,6+b/d,c/6+e,6+f"<<std::endl;
	std::cout<<"   -m     Ignore the middle layer shape."<<std::endl;
	std::cout<<"   -b     Use brackets in output around layer turns"<<std::endl;
	std::cout<<"   -r<n>  Solve n random positions, or infinitely many if n is 0 or missing."<<std::endl;
	std::cout<<"   -v<n>  Set verbosity, between 0 (minimal output) to 7 (full output)"<<std::endl;
	std::cout<<"   -h     Show this help"<<std::endl;
	std::cout<<"   -g     Input/Output generating move sequences rather than solutions."<<std::endl;
	std::cout<<"   -i<fn> Use as input each line from the file with filename <fn>."<<std::endl;
	std::cout<<"   -2     2gen - no bottom layer moves."<<std::endl;
	std::cout<<"   -p     Pseudo 2gen - only allow bottom layer moves of 1, 0, -1."<<std::endl;
	std::cout<<"   -c     Only generate algs that stay in a square/square cubeshape."<<std::endl;
	std::cout<<"   -k     Output algs in Karnotation. Ignores ABF."<<std::endl;
}


// -w|u=twist/turn metric  -a=all  -m=ignore middle
int main(int argc, char* argv[]){
	clock_t now = clock();
	bool ignoreMid=false;
	bool ignoreTrans=false;
	bool turnMetric=true;
	bool findAll=false;
	int twoGen = 0; // 0 = false, 1 = pseudo 2gen, 2 = true 2gen
	int numpos = -1;
	char *inpFile=NULL;
	int posArg=-1;
	usenegative=true; // why would you not want negative turns?
	int extraMoves = 0;
	bool keepCubeShape = false;
	for( int i=1; i<argc; i++){
		if( argv[i][0]=='-' ){
			switch( argv[i][1] ){
				case 'w':
				case 'W':
					turnMetric=false; break;
				case 'x':
				case 'X':
					ignoreTrans=true; break;
				case 'a':
				case 'A':
					findAll=true;
					extraMoves = parseInteger( argv[i]+2 );
					if( extraMoves<0 ) return show(15);
					break;
				case 'm':
				case 'M':
					ignoreMid=true; break;
				case 'n':
				case 'N':
					// "use negatives" flag has been removed
					break;
				case 'b':
				case 'B':
					usebrackets=true; break;
				case 'r':
				case 'R':
					numpos = parseInteger( argv[i]+2 );
					if( numpos<0 ) return show(15);
					break;
				case 'v':
				case 'V':
					verbosity = parseInteger( argv[i]+2 );
					if( verbosity<0 ) return show(15);
					break;
				case 'h':
				case 'H':
					help();
					return 0;
				case 'g':
				case 'G':
					generator = true;
					break;
				case 'i':
				case 'I':
					inpFile=argv[i]+2; break;
				case 'p':
				case 'P':
					twoGen = 1;
					break;
				case '2':
					twoGen = 2;
					break;
				case 'c':
				case 'C':
					keepCubeShape = true;
					break;
				case 'k':
				case 'K':
					karnotation = true;
					break;
				default:
					return show(1);
			}
		}else if( posArg<0 ){
			posArg = i;
		}else{
			return show(2);
		}
	}
	
	if (twoGen == 2 && keepCubeShape) return show(18);

	FullPosition p;
	std::ifstream is;
	// parse position/move sequence from argument posArg
	if( posArg>=0 ){
		int r=p.parseInput(argv[posArg]);
		if(r) return show(r);
	}else if( inpFile!=NULL ){
		is.open(inpFile);
		if(is.fail()) return show(3);
	}else if( numpos<0 ){
		help();
		return 0;
	}

	// now we have a position p to solve

	if(verbosity>=3) std::cout << "Initialising..."<<std::endl;
	// calculate transition tables
	ChoiceTable ct;
	if(verbosity>=4) std::cout << "  5. Shape transition table"<<std::endl;
	ShapeTranTable st;
	if(verbosity>=4) std::cout << "  4. Colouring 1 transition table"<<std::endl;
	ShpColTranTable scte( st, ct, true );
	if(verbosity>=4) std::cout << "  3. Colouring 2 transition table"<<std::endl;
	ShpColTranTable sctc( st, ct, false );

	//calculate pruning tables for two colourings
	FullPosition q;
	if(verbosity>=4) std::cout << "  2. Colouring 1 pruning table"<<std::endl;
	PrunTable pr1(q, 0, st,scte,sctc, turnMetric );
	if(verbosity>=4) std::cout << "  1. Colouring 2 pruning table"<<std::endl;
	PrunTable pr2(q, 1, st,scte,sctc, turnMetric );
	if(verbosity>=4) std::cout << "  0. Finished."<<std::endl;
	PositionSolver ps( st, scte, sctc, pr1, pr2 );
	PartialPositionSolver pps( st, scte, sctc, pr1, pr2 );

	if(verbosity>=2){
		std::cout<<"Flags: "<<(turnMetric? "Turn":"Twist")<<" Metric, ";
		std::cout<<"Find "<< (findAll? "every ":"first ");
		std::cout<< (generator? "generator":"solution");
		if (twoGen == 1) {
			std::cout << ", Pseudo 2gen";
		} else if (twoGen == 2) {
			std::cout << ", 2gen";
		}
		if (keepCubeShape) {
			std::cout << ", Keep Cube Shape";
		}
		std::cout<< std::endl;
	}

	srand( (unsigned)time( NULL ) );
	char buffer[2000];
	do{
		if( posArg<0 ){
			if( inpFile!=NULL ){
				is.getline(buffer,1999);
				int r=p.parseInput(buffer);
				if(r) {
					show(r);
					continue;
				}
			}else{
				p.random(twoGen, keepCubeShape);
			}
		}
		if( ignoreMid ) p.middle=0;

		//show position
		if(verbosity>=1){
			std::cout<<"Position: ";
			p.print();
			std::cout<<std::endl;
		}

		if (p.isPartial()) {
			// convert position to colour encoding
			pps.set(p, turnMetric, findAll, ignoreTrans);

			//solve position
			int r = pps.solve(twoGen, extraMoves, keepCubeShape);
			if (r) show(r);
			std::cout<<std::endl;
		} else {
			// convert position to colour encoding
			ps.set(p, turnMetric, findAll, ignoreTrans);

			//solve position
			int r = ps.solve(twoGen, extraMoves, keepCubeShape);
			if (r) show(r);
			std::cout<<std::endl;
		}
	}while( posArg<0 && ( (inpFile!=NULL && !is.eof() ) || (inpFile==NULL && (numpos==0 || numpos-- > 1)) ));

	std::cout << "Time: " << (clock() - now);
	return(0);
}





/*

ttshp: 7356*3 ints.   done

tt: 70*7356*3 chars for edges
tt: 70*7356*3 chars for corners

pt: 70*70*7356 chars colour 1,2
pt: 70*70*7356 chars colour 3

*/


