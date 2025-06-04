#include "MapChipField.h"
#include <assert.h>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace {

std::map<std::string, MapChipType> mapchipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock}
};

}

void MapChipField::ResetMapChipData() {

	// マップチップデータをリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);

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
	for (uint32_t i = 0; i < kNumBlockVirtical; i++) {
		std::string line;
		std::getline(mapChipCsv, line);

		//1行分の文字列をストリームに変換して解析しやすくする
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

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) { 
	if (xIndex<0||kNumBlockHorizontal-1<xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex<0||kNumBlockVirtical-1<yIndex) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0); }
