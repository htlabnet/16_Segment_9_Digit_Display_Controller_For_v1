//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	hexSegClient.ino
//	16 segment 9 digit displayをarduinoから制御するための"サンプル"
//
//	Created by thotgamma. (http://lab.thotgamma.com)
//
//	16Segment9DigitDisplay公式:
//		https://htlab.net/products/electronics/16-segment-9-digit-display-board-1/
//		https://htlab.net/products/electronics/16-segment-9-digit-display-controller-1/
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=




//初期化関数。シリアル通信を開始することが必要ということを
//ユーザーに知らせるための関数です。
void hexSegClientInit(){
	Serial.begin(9600);
}

//16セグに表示する文字を「1つずつ」指定して
//設定するための関数
void setHexSegChar(int digit, char character){
	switch(digit){
		case 0:
			Serial.write(0b11100000);
			break;
		case 1:
			Serial.write(0b11100001);
			break;
		case 2:
			Serial.write(0b11100010);
			break;
		case 3:
			Serial.write(0b11100011);
			break;
		case 4:
			Serial.write(0b11100100);
			break;
		case 5:
			Serial.write(0b11100101);
			break;
		case 6:
			Serial.write(0b11100110);
			break;
		case 7:
			Serial.write(0b11100111);
			break;
		case 8:
			Serial.write(0b11101000);
			break;
		default:
			break;
	}
	Serial.write(character);
}

//setHexSegChar()とほとんど変わりありませんが、こちらには
//「.」を表示する旨のコマンドが含まれています。
void setHexSegCharWithDot(int digit, char character){
	switch(digit){
		case 0:
			Serial.write(0b11110000);
			break;
		case 1:
			Serial.write(0b11110001);
			break;
		case 2:
			Serial.write(0b11110010);
			break;
		case 3:
			Serial.write(0b11110011);
			break;
		case 4:
			Serial.write(0b11110100);
			break;
		case 5:
			Serial.write(0b11110101);
			break;
		case 6:
			Serial.write(0b11110110);
			break;
		case 7:
			Serial.write(0b11110111);
			break;
		case 8:
			Serial.write(0b11111000);
			break;
		default:
			break;
	}
	Serial.write(character);
}

//任意の文字列を投げると適当に表示してくれる関数です。
//「HTLAB.NET!」など、文中に「.」が含まれていてもよしなに処理してくれます。
void setHexSegStr(String input){
	String buff = "";
	int dotFlag[input.length()];

	for(int i = 0; i < input.length(); i++){
		if(input[i] == '.'){
			if(i == 0){
				buff += ' ';
			}
			dotFlag[buff.length()-1] = 1;
		}else{
			buff += input[i];
			dotFlag[buff.length()-1] = 0;
		}
	}

	for(int i = 0; i < 9 || i < buff.length(); i++){
		if(dotFlag[i] == 0){
			setHexSegChar(i, buff[i]);
		}else{
			setHexSegCharWithDot(i, buff[i]);
		}
	}
}


//お好きに。
void setup() {
	// put your setup code here, to run once:
	hexSegClientInit();
	sample1("THIS IS 16 SEGMENT 9 DIGIT DISPLAY CREATED BY HTLABNET");

}

//定型文表示の場合。ループで表示文を投げておくことをお勧めします。
void loop() {
	//setHexSegStr("HTLAB.NET!");   
}


//適当なサンプルです。長い文字列を入れると適当に流して表示してくれます。
//ただし、「.」に関する適切な処理がなされていません。
void sample1(String input){

	while(true){
		for(int i = 0; i < input.length()-9; i++){
			setHexSegStr(input.substring(i,i+9));
			delay(1000);
		}
	}
}

