/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_STORAGE_IMPL_HPP
#define IROHA_STORAGE_IMPL_HPP

#include "ametsuchi/storage.hpp"

#include <atomic>
#include <cmath>
#include <shared_mutex>

#include <soci/soci.h>
#include <boost/optional.hpp>
#include "ametsuchi/block_storage_factory.hpp"
#include "ametsuchi/impl/postgres_options.hpp"
#include "ametsuchi/key_value_storage.hpp"
#include "ametsuchi/reconnection_strategy.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "interfaces/iroha_internal/block_json_converter.hpp"
#include "interfaces/permission_to_string.hpp"
#include "logger/logger_fwd.hpp"
#include "logger/logger_manager_fwd.hpp"

namespace iroha {
  namespace ametsuchi {

    class FailoverCallbackFactory;

    class StorageImpl : public Storage {
     protected:
      static expected::Result<bool, std::string> createDatabaseIfNotExist(
          const std::string &dbname,
          const std::string &options_str_without_dbname);

      static expected::Result<std::shared_ptr<soci::connection_pool>,
                              std::string>
      initPostgresConnection(std::string &options_str, size_t pool_size);

     public:
      static expected::Result<std::shared_ptr<StorageImpl>, std::string> create(
          std::shared_ptr<KeyValueStorage> block_store,
          std::string postgres_connection,
          std::shared_ptr<shared_model::interface::CommonObjectsFactory>
              factory,
          std::shared_ptr<shared_model::interface::BlockJsonConverter>
              converter,
          std::shared_ptr<shared_model::interface::PermissionToString>
              perm_converter,
          std::unique_ptr<BlockStorageFactory> block_storage_factory,
          std::unique_ptr<ReconnectionStrategyFactory>
              reconnection_strategy_factory,
          logger::LoggerManagerTreePtr log_manager,
          size_t pool_size = 10);

      expected::Result<std::unique_ptr<TemporaryWsv>, std::string>
      createTemporaryWsv() override;

      expected::Result<std::unique_ptr<MutableStorage>, std::string>
      createMutableStorage() override;

      boost::optional<std::shared_ptr<PeerQuery>> createPeerQuery()
          const override;

      boost::optional<std::shared_ptr<BlockQuery>> createBlockQuery()
          const override;

      boost::optional<std::shared_ptr<QueryExecutor>> createQueryExecutor(
          std::shared_ptr<PendingTransactionStorage> pending_txs_storage,
          std::shared_ptr<shared_model::interface::QueryResponseFactory>
              response_factory) const override;

      bool insertBlock(
          std::shared_ptr<const shared_model::interface::Block> block) override;

      expected::Result<void, std::string> insertPeer(
          const shared_model::interface::Peer &peer) override;

      expected::Result<std::unique_ptr<MutableStorage>, std::string>
      createMutableStorage(BlockStorageFactory &storage_factory) override;

      void reset() override;

      expected::Result<void, std::string> resetWsv() override;

      void resetPeers() override;

      void dropStorage() override;

      void freeConnections() override;

      boost::optional<std::unique_ptr<LedgerState>> commit(
          std::unique_ptr<MutableStorage> mutable_storage) override;

      boost::optional<std::unique_ptr<LedgerState>> commitPrepared(
          std::shared_ptr<const shared_model::interface::Block> block) override;

      std::shared_ptr<WsvQuery> getWsvQuery() const override;

      std::shared_ptr<BlockQuery> getBlockQuery() const override;

      rxcpp::observable<std::shared_ptr<const shared_model::interface::Block>>
      on_commit() override;

      void prepareBlock(std::unique_ptr<TemporaryWsv> wsv) override;

      ~StorageImpl() override;

     protected:
      StorageImpl(PostgresOptions postgres_options,
                  std::shared_ptr<KeyValueStorage> block_store,
                  std::shared_ptr<soci::connection_pool> connection,
                  std::shared_ptr<shared_model::interface::CommonObjectsFactory>
                      factory,
                  std::shared_ptr<shared_model::interface::BlockJsonConverter>
                      converter,
                  std::shared_ptr<shared_model::interface::PermissionToString>
                      perm_converter,
                  std::unique_ptr<BlockStorageFactory> block_storage_factory,
                  std::unique_ptr<ReconnectionStrategyFactory>
                      reconnection_strategy_factory,
                  size_t pool_size,
                  bool enable_prepared_blocks,
                  logger::LoggerManagerTreePtr log_manager);

      // db info
      const PostgresOptions postgres_options_;

     private:
      /**
       * revert prepared transaction
       */
      void rollbackPrepared(soci::session &sql);

      /**
       * add block to block storage
       */
      bool storeBlock(
          std::shared_ptr<const shared_model::interface::Block> block);

      std::shared_ptr<KeyValueStorage> block_store_;

      std::shared_ptr<soci::connection_pool> connection_;

      std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory_;

      rxcpp::composite_subscription notifier_lifetime_;
      rxcpp::subjects::subject<
          std::shared_ptr<const shared_model::interface::Block>>
          notifier_;

      std::shared_ptr<shared_model::interface::BlockJsonConverter> converter_;

      std::shared_ptr<shared_model::interface::PermissionToString>
          perm_converter_;

      std::unique_ptr<BlockStorageFactory> block_storage_factory_;

      logger::LoggerManagerTreePtr log_manager_;
      logger::LoggerPtr log_;

      mutable std::shared_timed_mutex drop_mutex;

      std::unique_ptr<ReconnectionStrategyFactory>
          reconnection_strategy_factory_;

      std::unique_ptr<FailoverCallbackFactory> callback_factory_;

      const size_t pool_size_;

      bool prepared_blocks_enabled_;

      std::atomic<bool> block_is_prepared;

      std::string prepared_block_name_;

     protected:
      static const std::string &drop_;
      static const std::string &reset_;
      static const std::string &reset_peers_;
      static const std::string &init_;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_STORAGE_IMPL_HPP
