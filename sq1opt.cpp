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
};

int verbosity = 5;
bool generator=false;
bool usenegative=false;
bool usebrackets=false;


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
class FullPosition {
	int pos[24];
public:
	int middle;
	FullPosition(){ reset(); }
	void reset(){
		middle=1;
		for( int i=0; i<24; i++)
			pos[i]="AAIBBJCCKDDLMEENFFOGGPHH"[i]-'A';
	}
	void print(){
		for(int i=0; i<24; i++){
			std::cout<<"ABCDEFGH12345678"[pos[i]];
			if( pos[i]<8 ) i++;
		}
		std::cout<<"/ -"[middle+1];
	}
	void random(int twoGen){
		middle = (rand()&1)!=0?-1:1;
		do{
			//mix array
			int tmp[16];
			int nToMix = twoGen==2 ? 12 : (twoGen==1 ? 13 : 16);
			for( int i=0; i<8; i++) {
				tmp[2*i + (i>3?1:0)] = i;
				tmp[2*i + (i>3?0:1)] = 8+i;
			}
			for( int i=0;i<nToMix; i++){
				int j=rand()%(nToMix-i);
				int k=tmp[i];tmp[i]=tmp[i+j];tmp[i+j]=k;
			}
			//store
			for(int i=0, j=0;i<16;i++){
				pos[j++]=tmp[i];
				if( tmp[i]<8 ) pos[j++]=tmp[i];
			}
			if (twoGen == 1 && (rand()&1)!=0 && pos[11]!=pos[12]) {
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
		int m=(cl!=2)?1<<7:1;
		for(int i=0; i<24; i++){
			if( pos[i]>=8 ){
				for(int j=0; j<4; j++){
					if( pos[i]==clp[cl][j] ){
						c|=m;
						break;
					}
				}
				if(cl!=2) m>>=1; else m<<=1;
			}
		}
		return(c);
	}
	int getCornerColouring(int cl){
		const int clp[3][4]={ {0,1,2,3}, {0,1,5,6}, {7,6,2,1} };
		int c=0;
		int m=(cl!=2)?1<<7:1;
		for(int i=0; i<24; i++){
			if( pos[i]<8 ){
				for(int j=0; j<4; j++){
					if( pos[i]==clp[cl][j] ){
						c|=m;
						break;
					}
				}
				if(cl!=2) m>>=1; else m<<=1;
				i++;
			}
		}
		return(c);
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
			}else if( (*t>='a' && *t<='h') || (*t>='A' && *t<='H') ){
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
			int j=0;
			int pi[24];
			for( int i=0; i<16; i++){
				int k=inp[i];
				if(k>='a' && k<='h') k-='a';
				else if(k>='A' && k<='H') k-='A';
				else if(k>='1' && k<='8') k-='1'-8;
				else return(10);
				pi[j++] = k;
				if( k<8) pi[j++] = k;
			}
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


// SimpPosition holds position encoded by colourings
class SimpPosition {
	int e0,e1,e2,c0,c1,c2;
	int shp,shp2,middle;
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

public:
	SimpPosition( ShapeTranTable& stt0, ShpColTranTable& scte0, ShpColTranTable& sctc0, PrunTable& pr10, PrunTable& pr20 )
		: stt(stt0), scte(scte0), sctc(sctc0), pr1(pr10), pr2(pr20) {};
	void set(FullPosition& p, bool turnMetric0, bool findAll0, bool ignoreTrans0){
		c0=sctc.ct.choice2Idx[p.getCornerColouring(0)];
		c1=sctc.ct.choice2Idx[p.getCornerColouring(1)];
		c2=sctc.ct.choice2Idx[p.getCornerColouring(2)];
		e0=scte.ct.choice2Idx[p.getEdgeColouring(0)];
		e1=scte.ct.choice2Idx[p.getEdgeColouring(1)];
		e2=scte.ct.choice2Idx[p.getEdgeColouring(2)];
		shp = stt.getShape(p.getShape(),p.getParityOdd());
		shp2 = stt.tranTable[shp][3];
		middle = p.middle;
		turnMetric=turnMetric0;
		findAll=findAll0;
		ignoreTrans=ignoreTrans0;
	};
	inline int doMove(int m){
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
	void solve(int twoGen, int extraMoves){
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
			int searchResult = search(l,3, &nodes, twoGen);
			if (searchResult != 0) {
				if (optimalMoves == -1) optimalMoves = l;
				if (l >= optimalMoves + extraMoves || (!turnMetric && middle!=0 && l+1 >= optimalMoves + extraMoves)) break;
			}
		};
	}
	int search( const int l, const int lm, unsigned long *nodes, int twoGen){
		int i,r=0;

		// search for l more moves. previous move was lm.
		(*nodes)++;
		if( l<0 ) return 0;

		//prune based on transformation
		// (a,b)/(c,d)/(e,f) -> (6+a,6+b)/(d,c)/(6+e,6+f)
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
			if( i<0 || ( i==0 && lastTurns[0]>=6 ) ) return(0);
		}

		// check if it is now solved
		if( l==0 ){
			if( shp==4163 && e0==69 && e1==44 && e2==44 && c0==69 && c1==44 && c2==44 && middle>=0 ){
				printsol();
				if(verbosity>=6) std::cout<<"Nodes="<<*nodes<<std::endl<<std::flush;
				return 1;
			}else if( turnMetric ) return 0;
		}

		// prune
		if( pr1.table[shp ][e0][c0]>l+1 ) return(0);
		if( pr2.table[shp ][e1][c1]>l+1 ) return(0);
		if( pr2.table[shp2][e2][c2]>l+1 ) return(0);

		// try all top layer moves
		if( lm>=2 ){
			i=doMove(0);
			do{
				if( turnMetric || ignoreTrans || twoGen!=0 || i<6 || l<2 ){
					moveList[moveLen++]=i;
					lastTurns[4]=i;
					r+=search( turnMetric?l-1:l, 0, nodes, twoGen);
					moveLen--;
					if(r!=0 && !findAll) return(r);
				}
				i+=doMove(0);
			}while( i<12);
			lastTurns[4]=0;
		}
		// try all bot layer moves
		if( lm!=1 && twoGen != 2){
			i=doMove(1);
			do{
				moveList[moveLen++]=i+12;
				lastTurns[5]=i;
				if (twoGen != 1 || i==1 || i==11) {
					r+=search( turnMetric?l-1:l, 1, nodes, twoGen);
				}
				moveLen--;
				if(r!=0 && !findAll) return(r);
				i+=doMove(1);
			}while( i<12);
			lastTurns[5]=0;
		}
		// try twist move
		if( lm!=2 ){
			int lt0=lastTurns[0], lt1=lastTurns[1];
			lastTurns[0]=lastTurns[2];
			lastTurns[1]=lastTurns[3];
			lastTurns[2]=lastTurns[4];
			lastTurns[3]=lastTurns[5];
			lastTurns[4]=0;
			lastTurns[5]=0;
			doMove(2);
			moveList[moveLen++]=0;
			r+=search(l-1, 2, nodes, twoGen);
			if(r!=0 && !findAll) return(r);
			moveLen--;
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
	void printmove(int mu, int md){
		if( mu!=0 || md!=0 ) {
			if( usebrackets ) std::cout<<"(";
			std::cout<<mu<<","<<md;
			if( usebrackets ) std::cout<<")";
		}
	}
	void printsol(){
		int tw=0, tu=0;
		int mu=0, md=0;
		if( generator ){
			for( int i=moveLen-1; i>=0; i--){
				if( moveList[i]==0 ){
					printmove(mu,md); mu = md = 0;
					std::cout<<"/";
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
					printmove(mu,md); mu = md = 0;
					std::cout<<"/";
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
		printmove(mu,md);
		std::cout <<"  ["<<tw<<"|"<<tu<<"] "<<std::endl;
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
	std::cout<<"   -2     Only search for solutions in 2gen (no bottom layer moves)."<<std::endl;
	std::cout<<"   -p     Pseudo 2gen - only allow bottom layer moves of 1, 0, -1."<<std::endl;
}


// -w|u=twist/turn metric  -a=all  -m=ignore middle
int main(int argc, char* argv[]){
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
				default:
					return show(1);
			}
		}else if( posArg<0 ){
			posArg = i;
		}else{
			return show(2);
		}
	}

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
	SimpPosition s( st, scte, sctc, pr1, pr2 );

	if(verbosity>=2){
		std::cout<<"Flags: "<<(turnMetric? "Turn":"Twist")<<" Metric, ";
		std::cout<<"Find "<< (findAll? "every ":"first ");
		std::cout<< (generator? "generator":"solution");
		if (twoGen == 1) {
			std::cout << ", Pseudo 2-gen";
		} else if (twoGen == 2) {
			std::cout << ", 2-gen";
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
				p.random(twoGen);
			}
		}
		if( ignoreMid ) p.middle=0;

		//show position
		if(verbosity>=1){
			std::cout<<"Position: ";
			p.print();
			std::cout<<std::endl;
		}

		// convert position to colour encoding
		s.set(p, turnMetric, findAll, ignoreTrans);

		//solve position
		s.solve(twoGen, extraMoves);
		std::cout<<std::endl;
	}while( posArg<0 && ( (inpFile!=NULL && !is.eof() ) || (inpFile==NULL && (numpos==0 || numpos-- > 1)) ));

	return(0);
}





/*

ttshp: 7356*3 ints.   done

tt: 70*7356*3 chars for edges
tt: 70*7356*3 chars for corners

pt: 70*70*7356 chars colour 1,2
pt: 70*70*7356 chars colour 3

*/


