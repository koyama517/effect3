#include <Novice.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

/*================================
	コピペ用↓
=================================*/

/*********************************
	大見出しコピペ
*********************************/

/******** 小見出しコピペ用 **********/

/*================================
	コピペ用↑
=================================*/

/******** ウィンドウ名の指定 **********/
const char kWindowTitle[] = "LC1A_16_トヨダユウキ_TD1_課題";

/******** ウィンドウサイズの指定 **********/
const int kWindowWidth = 1920; //x
const int kWindowHeight = 1080; //y

/*********************************
	定数の宣言ここまで
*********************************/

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	/*********************************
		構造体宣言ここから
	*********************************/

	/******** 二次元ベクトル **********/

	typedef struct vector2 {

		float x; //x
		float y; //y

	};

	/******** 距離 **********/

	typedef struct Distance {

		vector2 distance; //距離xy
		float length; //実際距離

	};

	/******** 画像 **********/

	typedef struct graph {

		vector2 translate;
		vector2 radius;
		vector2 imgRadius;
		vector2 drawStartArea;
		vector2 drawEndArea;
		int name;

	};

	/******** キャラクター **********/

	typedef struct chara {

		vector2 translate; //位置
		vector2 moveDirection; //動作方向
		float vectorLength; //ベクトル長さ

		float radius; //当たり判定半径

		float speed; //スピード
		float defSpeed; //スピードデフォルト/

		int HP; //キャラクタHP

		int damage; //与えるダメージ

		int graph; //キャラ画像
		int graphRadius; //画像半径

	};

	/******** エフェクト(残像) **********/

	typedef struct trajectory {
		vector2 translate; //位置
		float radius;
		unsigned int carentAlpha;
		int isActive;
		int graph;
		int graphRadius; //画像半径
	};

	/******** エフェクト(チャージ) **********/
	typedef struct effect {
		vector2 translate; //位置
		vector2 moveDirection; //動作方向
		float vectorLength; //ベクトル長さ
		float radius;
		unsigned int carentAlpha;
		int isActive;
		int graph;
		int graphRadius; //画像半径
	};

	/*********************************
		構造体宣言ここまで
	*********************************/

	/*********************************
		変数宣言ここから
	*********************************/

	/******** シーン **********/
	enum Scene {

		TITLE,
		GAME,
		RESULT

	};

	int scenes = TITLE;

	/******** ワールド座標原点 **********/
	vector2 worldPosOrigin{

		0, //x
		kWindowHeight //y

	};

	/******** スクロール **********/

	//開始座標
	vector2 scroolStartPos{

		kWindowWidth - kWindowWidth / 2, //x
		worldPosOrigin.y - kWindowHeight + kWindowHeight / 2 //y

	};

	//スクロール値
	vector2 scrool{

		0.0f, //x
		0.0f //y

	};

	/******** 背景 **********/

	int white1x1Png = Novice::LoadTexture("white1x1.png");

	int bgGraph[6];

	bgGraph[0] = Novice::LoadTexture("./resources/graph/map/BG_1.png");
	bgGraph[1] = Novice::LoadTexture("./resources/graph/map/BG_2.png");
	bgGraph[2] = Novice::LoadTexture("./resources/graph/map/BG_3.png");
	bgGraph[3] = Novice::LoadTexture("./resources/graph/map/BG_4.png");
	bgGraph[4] = Novice::LoadTexture("./resources/graph/map/BG_5.png");
	bgGraph[5] = Novice::LoadTexture("./resources/graph/map/BG_6.png");

	graph bg[6];

	for (int i = 0; i < 6; i++) {

		bg[i] = {

			kWindowWidth / 2, kWindowHeight / 2,
			kWindowWidth, kWindowHeight,
			kWindowWidth, kWindowHeight,
			0.0f, 0.0f,
			1920.0f, 1080.0f,
			bgGraph[i]

		};

	}

	/******** プレイヤー **********/
	chara player{

		kWindowWidth * 1.5f, kWindowHeight, //translate x, y
		0.0f, 0.0f, //moveDirection x, y
		0.0f, //vectorLength

		128.0f, //radius

		5.0f, //speed
		5.0f, //defSpeed

		1, //HP

		10, //damage

		Novice::LoadTexture("./resources/graph/player/player.png"), //graph
		128 //graphRadius

	};

	/******** チャージエフェクト **********/
	const int chargeEffectMax = 16;
	effect chargeEffect[chargeEffectMax];

	for (int i = 0; i < chargeEffectMax; i++)
	{
		chargeEffect[i].isActive = false;
		chargeEffect[i].vectorLength = player.radius * 2;
		chargeEffect[i].moveDirection.x = 0;
		chargeEffect[i].moveDirection.y = 0;
		chargeEffect[i].translate.x = chargeEffect[i].moveDirection.x * chargeEffect[i].vectorLength;
		chargeEffect[i].translate.y = chargeEffect[i].moveDirection.y * chargeEffect[i].vectorLength;
		chargeEffect[i].carentAlpha = 0xBA;
		chargeEffect[i].radius = 96;
		chargeEffect[i].graph = Novice::LoadTexture("./resources/graph/effect/effect.png");
		chargeEffect[i].graphRadius = 512;
	};

	/******** エフェクト(残像) **********/
	const int afterimageMax = 30;

	trajectory afterimage[afterimageMax];

	for (int i = 0; i < afterimageMax; i++)
	{
		afterimage[i].isActive = false;
		afterimage[i].translate.x = player.translate.x;
		afterimage[i].translate.y = player.translate.y;
		afterimage[i].carentAlpha = 0xBA;
		afterimage[i].radius = 128 / 2;
		afterimage[i].graph = Novice::LoadTexture("./resources/graph/player/player.png");
		afterimage[i].graphRadius = 128;
	};

	float theta = 0.0f;

	//スペースキートリガー用
	int isPressSpace = false;

	//チャージできるか
	int canCharge = true;
	//現在チャージしているか
	int isCharging = false;
	//チャージが完了しているか
	int compCharge = false;

	//攻撃中か
	int isAttacking = false;

	//チャージされているパワー
	float chargePower = 0.0f;
	//パワーの最大値
	float maxPower = 60.0f;

	//チャージ可能までのクールタイム
	float chargeCoolTime = 0.0f;
	//チャージ可能までのクールタイムのデフォルト値
	float defChargeCoolTime = 120.0f;

	unsigned int effectTime = time(nullptr);
	srand(effectTime);

	/*********************************
		変数宣言ここまで
	*********************************/

	/*********************************
		関数宣言ここから
	*********************************/

	/******** リソースロード **********/

	void LoadTitleResource();


	/*********************************
		関数宣言ここまで
	*********************************/

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		/*********************************
			更新処理ここから
		*********************************/

		/*================================
			コピペ用↓
		=================================*/

		/*********************************
			大見出しコピペ
		*********************************/

		/******** 小見出しコピペ用 **********/

		/*================================
			コピペ用↑
		=================================*/

		/*********************************
			スクリーン関係ここから
		*********************************/

		/******** フルスクリーン **********/
		Novice::SetWindowMode(kFullscreen);

		/*********************************
			スクリーン関係ここまで
		*********************************/

		switch (scenes)
		{
		case TITLE:

			if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {

				scenes++;

			}

			break;

		case GAME:

			/*********************************
				プレイヤー関係ここから
			*********************************/

			/******** プレイヤー移動 **********/

			//ベクトル初期化
			player.moveDirection.x = 0.0f;
			player.moveDirection.y = 0.0f;

			//移動方向指定
			if (keys[DIK_UP] || keys[DIK_W]) {

				player.moveDirection.y++; //上
			}

			if (keys[DIK_LEFT] || keys[DIK_A]) {

				player.moveDirection.x--; //左

			}

			if (keys[DIK_DOWN] || keys[DIK_S]) {

				player.moveDirection.y--; //下

			}

			if (keys[DIK_RIGHT] || keys[DIK_D]) {

				player.moveDirection.x++; //右

			}

			if (isCharging == false && isAttacking == false) {

				//ベクトルの長さを求める
				player.vectorLength = sqrt(pow(player.moveDirection.x, 2) + pow(player.moveDirection.y, 2));

				//ベクトルの長さが0以外の時
				if (player.vectorLength != 0.0f) {

					//x方向の移動
					player.translate.x += player.moveDirection.x / player.vectorLength * player.speed;

					//y方向の移動
					player.translate.y += player.moveDirection.y / player.vectorLength * player.speed;

				}
				else {

					//x方向の移動
					player.translate.x += player.moveDirection.x * player.speed;

					//y方向の移動
					player.translate.y += player.moveDirection.y * player.speed;

				}
			}

			if (player.translate.x <= 0 + player.radius / 2) {

				player.translate.x = player.radius / 2;

			}
			else if (player.translate.x >= kWindowWidth * 3 - player.radius / 2) {

				player.translate.x = kWindowWidth * 3 - player.radius / 2;

			}

			if (player.translate.y <= 0 + player.radius / 2) {

				player.translate.y = player.radius / 2;

			}
			else if (player.translate.y >= kWindowHeight * 2 - player.radius / 2) {

				player.translate.y = kWindowHeight * 2 - player.radius / 2;

			}

			/******** チャージ関係の処理 **********/

			//スペースキー長押し
			if (keys[DIK_SPACE]) {

				//チャージ状態true
				isCharging = true;

			}
			else {

				//チャージ状態false
				isCharging = false;

			}

			//チャージ状態trueの時
			if (isCharging == true && isAttacking == false) {

				if (chargePower < maxPower) {

					//チャージ
					chargePower += 2.0f;

				}
				else {

					//一定の値を超えたら固定
					chargePower = maxPower;

				}

				if (keys[DIK_RIGHT] || keys[DIK_D]) {

					theta += 0.1f;

				}

				else if (keys[DIK_LEFT] || keys[DIK_A]) {

					theta -= 0.1f;

				}

				/******** チャージエフェクト **********/
				for (int i = 0; i < chargeEffectMax; i++)
				{
					//エフェクトが出ていない
					if (!chargeEffect[i].isActive)
					{
						chargeEffect[i].translate.x = player.translate.x;
						chargeEffect[i].translate.y = player.translate.y;
						chargeEffect[i].isActive = true;
						chargeEffect[i].carentAlpha = 0xAA;
						chargeEffect[i].vectorLength = player.radius;
						if (rand() % 8 == 0)
						{
							chargeEffect[i].moveDirection.x = 0;
							chargeEffect[i].moveDirection.y = -1;
						}
						else if (rand() % 8 == 1) {
							chargeEffect[i].moveDirection.x = -1;
							chargeEffect[i].moveDirection.y = -1;
						}
						else if (rand() % 8 == 2) {
							chargeEffect[i].moveDirection.x = 1;
							chargeEffect[i].moveDirection.y = 0;
						}
						else if (rand() % 8 == 3) {
							chargeEffect[i].moveDirection.x = -1;
							chargeEffect[i].moveDirection.y = 1;
						}
						else if (rand() % 8 == 4) {
							chargeEffect[i].moveDirection.x = 1;
							chargeEffect[i].moveDirection.y = 1;
						}
						else if (rand() % 8 == 5) {
							chargeEffect[i].moveDirection.x = 1;
							chargeEffect[i].moveDirection.y = -1;
						}
						else if (rand() % 8 == 6) {
							chargeEffect[i].moveDirection.x = -1;
							chargeEffect[i].moveDirection.y = 0;
						}
						else if (rand() % 8 == 7) {
							chargeEffect[i].moveDirection.x = 0;
							chargeEffect[i].moveDirection.y = 1;
						}
						break;
					}
					//エフェクトが出ている
					else {
						chargeEffect[i].vectorLength -= 4;
						if (chargeEffect[i].vectorLength <= 0) {
							chargeEffect[i].isActive = false;
						}
					}

				}

			}
			else {

				if (chargePower > 0) {

					//スペースキーを離したらパワーが減る
					chargePower -= 4.0f;

				}
				else {

					//0以下になったら値を0にリセット
					chargePower = 0;

					isAttacking = false;

				}

				//動いた時にエフェクトを薄くしていく
				for (int i = 0; i < chargeEffectMax; i++)
				{
					if (chargeEffect[i].isActive)
					{
						chargeEffect[i].carentAlpha -= 0x11;
						if (chargeEffect[i].carentAlpha <= 0) {
							chargeEffect[i].isActive = false;
						}
					}
				}

			}

			if (theta >= 6.0f) {

				theta = 0.0f;

			}
			else if (theta < 0.0f) {

				theta = 5.9f;

			}

			if (!keys[DIK_SPACE] && preKeys[DIK_SPACE]) {

				if (isAttacking == false) {

					isAttacking = true;

				}

			}

			if (isAttacking == true) {

				player.translate.x += (cosf(theta) * player.speed * chargePower / 3.0f);
				player.translate.y += -(sinf(theta) * player.speed * chargePower / 3.0f);

				chargePower--;

				//残像
				for (int i = 0; i < afterimageMax; i++)
				{
					//残像が出ていない
					if (!afterimage[i].isActive)
					{
						afterimage[i].isActive = true;
						afterimage[i].translate.x = player.translate.x;
						afterimage[i].translate.y = player.translate.y;
						afterimage[i].carentAlpha = 0xBA;
						afterimage[i].radius = 128;
						break;
					}
					//出てる
					else
					{
						afterimage[i].carentAlpha -= 0x06;
						if (afterimage[i].carentAlpha <= 0x00) {
							afterimage[i].isActive = false;
						}
					}
				}
			}
			//動いてないとき
			else
			{
				for (int i = 0; i < afterimageMax; i++)
				{
					if (afterimage[i].isActive)
					{
						afterimage[i].carentAlpha -= 0x06;
						if (afterimage[i].carentAlpha <= 0x00) {
							afterimage[i].isActive = false;
						}
					}
				}
			}

			/*********************************
				プレイヤー関係ここから
			*********************************/

			/*********************************
				スクロール関係ここから
			*********************************/

			/******** スクロール処理 **********/

			if (player.translate.x >= scroolStartPos.x || player.translate.x <= scroolStartPos.x) {

				scrool.x = player.translate.x - scroolStartPos.x;

			}
			else {

				scrool.x = 0.0f;

			}

			if (player.translate.y >= scroolStartPos.y || player.translate.y <= scroolStartPos.y) {

				scrool.y = player.translate.y - scroolStartPos.y;

			}
			else {

				scrool.y = 0.0f;

			}

			/*********************************
				スクロール関係ここまで
			*********************************/

			/*********************************
				更新処理ここまで
			*********************************/

			break;

		case RESULT:



			break;

		}

		/*********************************
			描画処理ここから
		*********************************/

		/*================================
			コピペ用↓
		=================================*/

		/*********************************
			大見出しコピペ
		*********************************/

		/******** 小見出しコピペ用 **********/

		/*================================
			コピペ用↑
		=================================*/

		Novice::DrawQuad(

			worldPosOrigin.x + player.translate.x - kWindowWidth / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y - kWindowHeight / 2 + scrool.y,

			worldPosOrigin.x + player.translate.x + kWindowWidth / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y - kWindowHeight / 2 + scrool.y,

			worldPosOrigin.x + player.translate.x - kWindowWidth / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y + kWindowHeight / 2 + scrool.y,

			worldPosOrigin.x + player.translate.x + kWindowWidth / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y + kWindowHeight / 2 + scrool.y,

			0,
			0,

			1,
			1,

			white1x1Png,
			BLACK

		);

		/******** 背景描画 **********/

		for (int i = 0; i < 3; i++) {


			Novice::DrawQuad(

				worldPosOrigin.x + bg[i].translate.x * (i + 1) + (kWindowWidth / 2 * i) - bg[i].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i].translate.y - bg[i].radius.y / 2 - (kWindowHeight)+scrool.y,

				worldPosOrigin.x + bg[i].translate.x * (i + 1) + (kWindowWidth / 2 * i) + bg[i].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i].translate.y - bg[i].radius.y / 2 - (kWindowHeight)+scrool.y,

				worldPosOrigin.x + bg[i].translate.x * (i + 1) + (kWindowWidth / 2 * i) - bg[i].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i].translate.y + bg[i].radius.y / 2 - (kWindowHeight)+scrool.y,

				worldPosOrigin.x + bg[i].translate.x * (i + 1) + (kWindowWidth / 2 * i) + bg[i].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i].translate.y + bg[i].radius.y / 2 - (kWindowHeight)+scrool.y,

				bg[i].drawStartArea.x,
				bg[i].drawStartArea.y,

				bg[i].drawEndArea.x,
				bg[i].drawEndArea.y,

				bg[i].name,
				WHITE

			);


		}

		for (int i = 0; i < 3; i++) {


			Novice::DrawQuad(

				worldPosOrigin.x + bg[i + 3].translate.x * (i + 1) + (kWindowWidth / 2 * i) - bg[i + 3].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i + 3].translate.y - bg[i + 3].radius.y / 2 + scrool.y,

				worldPosOrigin.x + bg[i + 3].translate.x * (i + 1) + (kWindowWidth / 2 * i) + bg[i + 3].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i + 3].translate.y - bg[i].radius.y / 2 + scrool.y,

				worldPosOrigin.x + bg[i + 3].translate.x * (i + 1) + (kWindowWidth / 2 * i) - bg[i + 3].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i + 3].translate.y + bg[i + 3].radius.y / 2 + scrool.y,

				worldPosOrigin.x + bg[i + 3].translate.x * (i + 1) + (kWindowWidth / 2 * i) + bg[i + 3].radius.x / 2 - scrool.x,
				worldPosOrigin.y - bg[i + 3].translate.y + bg[i + 3].radius.y / 2 + scrool.y,

				bg[i + 3].drawStartArea.x,
				bg[i + 3].drawStartArea.y,

				bg[i + 3].drawEndArea.x,
				bg[i + 3].drawEndArea.y,

				bg[i + 3].name,
				WHITE

			);


		}

		/******** 残像 **********/
		for (int i = 0; i < afterimageMax; i++)
		{
			if (afterimage[i].isActive)
			{
				Novice::DrawQuad(

					worldPosOrigin.x + afterimage[i].translate.x - afterimage[i].radius / 2 - scrool.x,
					worldPosOrigin.y - afterimage[i].translate.y - afterimage[i].radius / 2 + scrool.y,

					worldPosOrigin.x + afterimage[i].translate.x + afterimage[i].radius / 2 - scrool.x,
					worldPosOrigin.y - afterimage[i].translate.y - afterimage[i].radius / 2 + scrool.y,

					worldPosOrigin.x + afterimage[i].translate.x - afterimage[i].radius / 2 - scrool.x,
					worldPosOrigin.y - afterimage[i].translate.y + afterimage[i].radius / 2 + scrool.y,

					worldPosOrigin.x + afterimage[i].translate.x + afterimage[i].radius / 2 - scrool.x,
					worldPosOrigin.y - afterimage[i].translate.y + afterimage[i].radius / 2 + scrool.y,

					0,
					0,

					afterimage[i].graphRadius,
					afterimage[i].graphRadius,

					afterimage[i].graph,
					0xFFFFFF00 + afterimage[i].carentAlpha

				);
			}
		}

		/******** チャージエフェクト **********/
		for (int i = 0; i < chargeEffectMax; i++)
		{
			if (chargeEffect[i].isActive)
			{
				Novice::DrawQuad(

					worldPosOrigin.x + chargeEffect[i].translate.x - chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.x - scrool.x,
					worldPosOrigin.y - chargeEffect[i].translate.y - chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.y + scrool.y,

					worldPosOrigin.x + chargeEffect[i].translate.x + chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.x - scrool.x,
					worldPosOrigin.y - chargeEffect[i].translate.y - chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.y + scrool.y,

					worldPosOrigin.x + chargeEffect[i].translate.x - chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.x - scrool.x,
					worldPosOrigin.y - chargeEffect[i].translate.y + chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.y + scrool.y,

					worldPosOrigin.x + chargeEffect[i].translate.x + chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.x - scrool.x,
					worldPosOrigin.y - chargeEffect[i].translate.y + chargeEffect[i].radius * 0.4f + chargeEffect[i].vectorLength * chargeEffect[i].moveDirection.y + scrool.y,

					0,
					0,

					chargeEffect[i].graphRadius,
					chargeEffect[i].graphRadius,

					chargeEffect[i].graph,
					0xFFFFFF00 + chargeEffect[i].carentAlpha

				);
			}
		}

		/******** プレイヤー描画 **********/
		Novice::DrawQuad(

			worldPosOrigin.x + player.translate.x - player.radius / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y - player.radius / 2 + scrool.y,

			worldPosOrigin.x + player.translate.x + player.radius / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y - player.radius / 2 + scrool.y,

			worldPosOrigin.x + player.translate.x - player.radius / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y + player.radius / 2 + scrool.y,

			worldPosOrigin.x + player.translate.x + player.radius / 2 - scrool.x,
			worldPosOrigin.y - player.translate.y + player.radius / 2 + scrool.y,

			0,
			0,

			player.graphRadius,
			player.graphRadius,

			player.graph,
			WHITE

		);

		/******** プレイヤーデバック描画 **********/

		//座標
		Novice::ScreenPrintf(0, 10, "Px : %4.2f Py : %4.2f", player.translate.x, player.translate.y);

		//スピード
		Novice::ScreenPrintf(0, 30, "PSx : %4.2f PSy : %4.2f", player.speed, player.defSpeed);

		//チャージされているパワー
		Novice::ScreenPrintf(0, 50, "CP : %4.2f", chargePower);

		Novice::ScreenPrintf(0, 70, "IsAt : %d", isAttacking);

		Novice::ScreenPrintf(0, 90, "scrX : %4.2f scrY : %4.2f", scrool.x, scrool.y);

		//発射方向
		Novice::DrawLine(
			worldPosOrigin.x + player.translate.x - scrool.x,
			worldPosOrigin.y - player.translate.y + scrool.y,
			worldPosOrigin.x + player.translate.x + (cosf(theta) * player.speed * 2 * chargePower / 3) - scrool.x,
			worldPosOrigin.y - player.translate.y + (sinf(theta) * player.speed * 2 * chargePower / 3) + scrool.y,
			WHITE
		);

		/********************************
			描画処理ここまで
		********************************/

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
