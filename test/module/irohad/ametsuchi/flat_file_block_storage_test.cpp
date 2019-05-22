/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/flat_file_block_storage.hpp"
#include "ametsuchi/impl/flat_file_block_storage_factory.hpp"

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include "framework/test_logger.hpp"
#include "module/shared_model/interface_mocks.hpp"

using namespace iroha::ametsuchi;
using namespace boost::filesystem;

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

class FlatFileBlockStorageTest : public ::testing::Test {
 public:
  FlatFileBlockStorageTest() {
    ON_CALL(*block_, height()).WillByDefault(Return(height_));
  }

 protected:
  void SetUp() override {
    create_directory(path_provider_());
    block_store_ = *FlatFile::create(
        path_provider_(), log_manager_->getChild("FlatFile")->getLogger());
  }

  const std::string block_store_path_ =
      (temp_directory_path() / unique_path()).string();

  std::function<std::string()> path_provider_ = [&]() {
    return block_store_path_;
  };

  std::shared_ptr<KeyValueStorage> block_store_;
  std::shared_ptr<MockBlockJsonConverter> converter_ =
      std::make_shared<NiceMock<MockBlockJsonConverter>>();
  logger::LoggerManagerTreePtr log_manager_ = getTestLoggerManager();
  std::shared_ptr<MockBlock> block_ = std::make_shared<NiceMock<MockBlock>>();
  shared_model::interface::types::HeightType height_ = 1;
};

/**
 * @given block storage factory
 * @when create is called
 * @then block storage is created
 */
TEST_F(FlatFileBlockStorageTest, Creation) {
  auto block_storage =
      FlatFileBlockStorageFactory(block_store_, converter_, log_manager_)
          .create();
  ASSERT_TRUE(block_storage);
}

/**
 * @given initialized block storage, single block with height_ inserted
 * @when another block with height_ is inserted
 * @then second insertion fails
 */
TEST_F(FlatFileBlockStorageTest, Insert) {
  auto block_storage =
      FlatFileBlockStorageFactory(block_store_, converter_, log_manager_)
          .create();
  ASSERT_TRUE(block_storage->insert(block_));
  ASSERT_FALSE(block_storage->insert(block_));
}

/**
 * @given initialized block storage, single block with height_ inserted
 * @when block with height_ is fetched
 * @then it is returned
 */
TEST_F(FlatFileBlockStorageTest, FetchExisting) {
  auto block_storage =
      FlatFileBlockStorageFactory(block_store_, converter_, log_manager_)
          .create();
  ASSERT_TRUE(block_storage->insert(block_));

  shared_model::interface::Block *raw_block;

  EXPECT_CALL(*converter_, deserialize(_))
      .WillOnce(Invoke([&](const shared_model::interface::types::JsonType &)
                           -> iroha::expected::Result<
                               std::unique_ptr<shared_model::interface::Block>,
                               std::string> {
        auto return_block = std::make_unique<MockBlock>();
        raw_block = return_block.get();
        return iroha::expected::makeValue<
            std::unique_ptr<shared_model::interface::Block>>(
            std::move(return_block));
      }));
  std::shared_ptr<const shared_model::interface::Block> block_var =
      *(block_storage->fetch(height_));

  ASSERT_EQ(raw_block, block_var.get());
}

/**
 * @given initialized block storage without blocks
 * @when block with height_ is fetched
 * @then nothing is returned
 */
TEST_F(FlatFileBlockStorageTest, FetchNonexistent) {
  auto block_storage =
      FlatFileBlockStorageFactory(block_store_, converter_, log_manager_)
          .create();
  auto block_var = block_storage->fetch(height_);
  ASSERT_FALSE(block_var);
}

/**
 * @given initialized block storage, single block with height_ inserted
 * @when size is fetched
 * @then 1 is returned
 */
TEST_F(FlatFileBlockStorageTest, Size) {
  auto block_storage =
      FlatFileBlockStorageFactory(block_store_, converter_, log_manager_)
          .create();
  ASSERT_TRUE(block_storage->insert(block_));

  ASSERT_EQ(1, block_storage->size());
}

/**
 * @given initialized block storage, single block with height_ inserted
 * @when storage is cleared with clear
 * @then no blocks are left in storage
 */
TEST_F(FlatFileBlockStorageTest, Clear) {
  auto block_storage =
      FlatFileBlockStorageFactory(block_store_, converter_, log_manager_)
          .create();
  ASSERT_TRUE(block_storage->insert(block_));

  block_storage->clear();

  auto block_var = block_storage->fetch(height_);

  ASSERT_FALSE(block_var);
}

/**
 * @given initialized block storage, single block with height_ inserted
 * @when commit is called
 * @then block with height_ is visited, lambda is invoked once,
 * block storage is empty after commit
 */
TEST_F(FlatFileBlockStorageTest, Commit) {
  auto block_storage =
      FlatFileBlockStorageFactory(block_store_, converter_, log_manager_)
          .create();
  ASSERT_TRUE(block_storage->insert(block_));

  shared_model::interface::Block *raw_block;

  EXPECT_CALL(*converter_, deserialize(_))
      .WillOnce(Invoke([&](const shared_model::interface::types::JsonType &)
                           -> iroha::expected::Result<
                               std::unique_ptr<shared_model::interface::Block>,
                               std::string> {
        auto return_block = std::make_unique<MockBlock>();
        raw_block = return_block.get();
        return iroha::expected::makeValue<
            std::unique_ptr<shared_model::interface::Block>>(
            std::move(return_block));
      }));

  size_t count = 0;

  block_storage->commit([&count, &raw_block](const auto &block) {
    ++count;
    ASSERT_EQ(raw_block, block.get());
  });

  ASSERT_EQ(1, count);

  ASSERT_EQ(0, block_storage->size());
}
