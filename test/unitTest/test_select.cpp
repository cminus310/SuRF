#include "gtest/gtest.h"

#include <assert.h>

#include <fstream>
#include <string>
#include <vector>

#include "config.hpp"
#include "select.hpp"
#include "surf_builder.hpp"

namespace surf {

namespace selecttest {

static const std::string kFilePath = "../../../test/words.txt";
static const int kTestSize = 234369;
static std::vector<std::string> words;

class SelectUnitTest : public ::testing::Test {
public:
    virtual void SetUp () {
	bool include_dense = false;
	uint32_t sparse_dense_ratio = 0;
	builder_ = new SuRFBuilder(include_dense, sparse_dense_ratio, kReal);
	bv_ = NULL;
	num_items_ = 0;
    }
    virtual void TearDown () {
	delete builder_;
	if (bv_ != NULL) delete bv_;
    }

    void setupWordsTest();

    static const position_t kSelectSampleInterval = 64;

    SuRFBuilder* builder_;
    BitvectorSelect* bv_;
    std::vector<position_t> num_items_per_level_;
    position_t num_items_;
};

void SelectUnitTest::setupWordsTest() {
    builder_->build(words);
    for (level_t level = 0; level < builder_->getTreeHeight(); level++)
	num_items_per_level_.push_back(builder_->getLabels()[level].size());
    for (level_t level = 0; level < num_items_per_level_.size(); level++)
	num_items_ += num_items_per_level_[level];
    bv_ = new BitvectorSelect(kSelectSampleInterval, builder_->getLoudsBits(), num_items_per_level_);
}

TEST_F (SelectUnitTest, readBitTest) {
    setupWordsTest();
    position_t bv_pos = 0;
    for (level_t level = 0; level < builder_->getTreeHeight(); level++) {
	for (position_t pos = 0; pos < num_items_per_level_[level]; pos++) {
	    bool expected_bit = SuRFBuilder::readBit(builder_->getLoudsBits()[level], pos);
	    bool bv_bit = bv_->readBit(bv_pos);
	    ASSERT_EQ(expected_bit, bv_bit);
	    bv_pos++;
	}
    }
}

TEST_F (SelectUnitTest, selectTest) {
    setupWordsTest();
    position_t rank = 1;
    for (position_t pos = 0; pos < num_items_; pos++) {
	if (bv_->readBit(pos)) {
	    position_t select = bv_->select(rank);
	    ASSERT_EQ(pos, select);
	    rank++;
	}
    }
}

void loadWordList() {
    std::ifstream infile(kFilePath);
    std::string key;
    int count = 0;
    while (infile.good() && count < kTestSize) {
	infile >> key;
	words.push_back(key);
	count++;
    }
}

} // namespace ranktest

} // namespace surf

int main (int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    surf::selecttest::loadWordList();
    return RUN_ALL_TESTS();
}
