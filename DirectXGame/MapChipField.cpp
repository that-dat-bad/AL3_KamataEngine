#include "MapChipField.h"
#include <assert.h>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace {

std::map<std::string, MapChipField::MapChipType> mapchipTable = {
    {"0", MapChipField::MapChipType::kBlank},
    {"1", MapChipField::MapChipType::kBlock}
};

}

void MapChipField::ResetMapChipData() {

	// マップチップデータをリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVertical);

	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// マップチップ
	std::stringstream mapChipCsv;

	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVertical; i++) {
		std::string line;
		std::getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくする
		std::stringstream lineStream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; j++) {
			std::string word;
			std::getline(lineStream, word, ',');
			if (mapchipTable.contains(word)) {
				mapChipData_.data[i][j] = mapchipTable[word];
			}
		}
	}
}

MapChipField::MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex < 0 || kNumBlockVertical - 1 < yIndex) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVertical - 1 - yIndex), 0); }

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);

	uint32_t tempYIndex = static_cast<uint32_t>((position.y + kBlockHeight / 2.0f) / kBlockHeight);
	indexSet.yIndex = static_cast<uint32_t>(kNumBlockVertical - 1 - tempYIndex);
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	// 指定ブロックの中心座標を取得
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f; //
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
}
