/*
 * Copyright (c) 2013-2014 John Connor (BM-NC49AxAjcqVcF5jNPu85Rb8MJ2d9JqZt)
 *
 * This file is part of coinpp.
 *
 * coinpp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COIN_WALLET_HPP
#define COIN_WALLET_HPP

#include <cstdint>
#include <list>
#include <map>
#include <mutex>
#include <set>

#include <coin/destination.hpp>
#include <coin/db_wallet.hpp>
#include <coin/key_public.hpp>
#include <coin/key_store_crypto.hpp>
#include <coin/key_wallet_master.hpp>
#include <coin/output.hpp>
#include <coin/sha256.hpp>
#include <coin/transaction.hpp>
#include <coin/transaction_in.hpp>
#include <coin/transaction_out.hpp>
#include <coin/transaction_wallet.hpp>
#include <coin/utility.hpp>

namespace coin {

    class accounting_entry;
    class block_locator;
    class coin_control;
    class key_pool;
    class key_reserved;
    
    /**
     * Implements a wallet.
     */
    class wallet : public key_store_crypto
    {
        public:
        
            /**
             * A transaction pair.
             */
            typedef std::pair<
                transaction_wallet *, accounting_entry *
            > tx_pair_t;
        
            /**
             * Holds transaction pairs.
             */
            typedef std::multimap<std::int64_t, tx_pair_t> tx_items_t;
        
            /**
             * Client version number for wallet features.
             */
            typedef enum
            {
                feature_base = 10500,
                feature_walletcrypt = 40000,
                feature_comprpubkey = 60000,
                feature_latest = 60000
            } feature_t;

            /**
             * Constructor
             * @param impl The stack_impl.
             */
            wallet(stack_impl & impl);
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
    
            /**
             * Flushes the wallet to disk.
             */
            void flush();
        
            /**
             * Unlocks the wallet.
             * @param passphrase The passphrase.
             */
            bool unlock(const std::string & passphrase);
        
            /**
             * Check whether we are allowed to upgrade (or already support) to
             * the named feature.
             */
            bool can_support_feature(const feature_t & value);
    
            /**
             * Generates a new key.
             */
            key_public generate_new_key();

            /**
             * Adds a key to the store, without saving it to disk.
             * @param k The key.
             */
            bool load_key(const key & k);
    
            /**
             * Loads the minimum version.
             * @param version The version.
             */
            bool load_minimum_version(const std::int32_t & version);

            /**
             * Adds a key.
             */
            bool add_key(const key & val);
        
            /**
             * Adds a encrypted key to the store and saves it to disk.
             */
            bool add_crypted_key(
                const key_public & pub_key,
                const std::vector<std::uint8_t> & crypted_secret
            );

            /**
             * Adds an encrypted key to the store, without saving it to disk.
             */
            bool load_crypted_key(
                const key_public & pub_key,
                const std::vector<std::uint8_t> & crypted_secret
            );
        
            /**
             * Adds a c script.
             * @param script_redeem The script.
             */
            bool add_c_script(const script & script_redeem);
        
            /**
             * Loads a c script.
             * @param script_redeem The script.
             */
            bool load_c_script(const script & script_redeem);
        
           /**
            * Increment and return the next transaction order id.
            * @param ptr_wallet_db The db_wallet.
            */
            std::int64_t increment_order_position_next(
                db_wallet * ptr_wallet_db = 0
            ) const;

            /**
             * Marks old keys as used and generate new ones.
             */
            bool new_key_pool();
    
            /**
             * Tops up the key pool.
             */
            bool top_up_key_pool();
        
            /**
             * Reserves a key from the key pool.
             * @param index The index.
             * @param keypool The key_pool.
             */
            void reserve_key_from_key_pool(
                std::int64_t & index, key_pool & keypool
            );
        
            /**
             * Removes a key from the key_pool.
             * @param index The index.
             */
            void keep_key(const std::int64_t & index);

            /**
             * Returns a key to the key_pool.
             * @param index The index.
             */
            void return_key(const std::int64_t & index);

            /**
             * Gets a key from the pool.
             * @param result The key_public.
             * @param allow_reuse If true we will reuse keys if necessary.
             */
            bool get_key_from_pool(
                key_public & result, const bool & allow_reuse = true
            );
        
            /**
             * If true it is from me.
             * @param tx The transaction.
             */
            bool is_from_me(const transaction & tx) const;
    
            /**
             * If true it is mine.
             * @param tx The transaction.
             */
            bool is_mine(const transaction & tx) const;
    
            /**
             * If true it is mine.
             * @param tx_in The transaction_in.
             */
            bool is_mine(const transaction_in & tx_in) const;
        
            /**
             * If true it is mine.
             * @param tx_out The transaction_out.
             */
            bool is_mine(const transaction_out & tx_out) const;
    
            /**
             * Gets debit.
             * @param tx The transaction.
             */
            std::int64_t get_debit(const transaction & tx) const;
    
            /**
             * Gets debit.
             * @param tx_in The transaction_in.
             */
            std::int64_t get_debit(const transaction_in & tx_in) const;

            /**
             * Gets credit.
             * @param tx_out The transaction_out.
             */
            std::int64_t get_credit(const transaction_out & tx_out) const;
        
            /**
             * Gets credit.
             * @param tx The transaction.
             */
            std::int64_t get_credit(const transaction & tx) const;
    
            /**
             * If true the transaction_out is change.
             * @param tx_out The transaction_out.
             */
            bool is_change(const transaction_out & tx_out) const;

            /**
             * Sets the best chain.
             * @param value The block_locator.
             */
            void set_best_chain(const block_locator & value);

            /**
             * Loads the wallet.
             * @param first_run If set to true this is the first run.
             */
            db_wallet::error_t load_wallet(bool & first_run);
        
            /**
             * Called when a transaction has been updated.
             * @param val The sha256.
             */
            void on_transaction_updated(const sha256 & val);

            /**
             * Called when inventory has changed.
             * @param val The sha256.
             */
            void on_inventory(const sha256 & val);
    
            /**
             * Erases the value from the wallet.
             * @param val The sha256.
             */
            bool erase_from_wallet(const sha256 & val) const;

            /**
             * Scans the wallet for transactions belonging to us.
             * @param index_start The block_index.
             * @param update If true the existing transactions will be
             * updated.
             */
            std::int32_t scan_for_transactions(
                const std::shared_ptr<block_index> & index_start,
                const bool & update = false
            );
        
            /**
             * Reaccepts wallet transactions
             */
            void reaccept_wallet_transactions();
        
            /**
             * Fixes spent coins.
             * @param mismatch_spent The mismatch spent.
             * @param The balance in question.
             * @param If true a check will be performed only.
             */
            void fix_spent_coins(
                std::int32_t & mismatch_spent,
                std::int64_t & balance_in_question,
                const bool & check_only = false
            );
    
            /**
             * Disable transaction (only for coinstake) (ppcoin).
             */
            void disable_transaction(const transaction & tx) const;

            /**
             * Add a transaction to the wallet, or update it.
             * @param tx The transaction.
             * @param blk The block.
             * @param update If true existing transactions will be updated.
             */
            bool add_to_wallet_if_involving_me(
                const transaction & tx, block * blk, const bool & update
            );

            /**
             * Returns the ordered transaction items.
             * @param entries The entries.
             * @param account The account.
             */
            tx_items_t ordered_tx_items(
                std::list<accounting_entry> & entries,
                const std::string & account = ""
            ) const;
        
            /**
             * Updates spent.
             * @param tx The transaction.
             */
            void update_spent(const transaction & tx) const;
        
            /**
             * Adds the transaction to the wallet.
             * @param wtx_in The transaction_wallet.
             */
            bool add_to_wallet(const transaction_wallet & wtx_in);
        
            /**
             * Sets the address book name.
             * @param addr The destination::tx_t.
             * @param name The name.
             */
            bool set_address_book_name(
                const destination::tx_t & addr, const std::string & name
            ) const;

            /**
             * Sets the default public key.
             * @param value The key_public.
             * @param write_to_disk If true the value will be written to disk.
             */
            bool set_key_public_default(
                const key_public & value, const bool & write_to_disk = false
            ) const;
        
            /**
             * The default public key.
             */
            const key_public & key_public_default() const;
        
            /**
             * The key pool.
             */
            std::set<std::int64_t> & get_key_pool();
        
            /**
             * The master keys.
             */
            std::map<std::uint32_t, key_wallet_master> & master_keys();
        
            /**
             * Sets the master key max id.
             */
            void set_master_key_max_id(const std::uint32_t & val);
        
            /**
             * The master key max id.
             */
            const std::uint32_t & master_key_max_id() const;
        
            /**
             * Set the minimim allowed version.
             */
            bool set_min_version(
                feature_t version, db_wallet * ptr_db_wallet = 0,
                const bool & explicit_upgrade = false
            );

            /**
             * Set which version we're allowed to upgrade to.
            */
            bool set_max_version(const std::int32_t & version);

            /**
             * The current wallet format (the oldest client version guaranteed
             * to understand this wallet).
             */
            std::int32_t get_version();
        
            /**
             * Gets the balance.
             */
            std::int64_t get_balance() const;
        
            /**
             * Gets the unconfirmed balance.
             */
            std::int64_t get_unconfirmed_balance() const;
        
            /**
             * Gets the unconfirmed balance.
             */
            std::int64_t get_immature_balance() const;

            /**
             * The total coins staked (non-spendable until maturity) (ppcoin).
             */
            std::int64_t get_stake() const;

            /**
             * Gets the new mint.
             */
            std::int64_t get_new_mint() const;
        
            /**
             * Selects coins.
             * @param target_value The target value.
             * @param spend_time The spend time.
             * @param coins_out The coins out.
             * @param value_out The value out.
             * @param control The coin_control.
             */
            bool select_coins(
                const std::int64_t & target_value,
                const std::uint32_t & spend_time,
                std::set< std::pair<transaction_wallet,
                std::uint32_t> > & coins_out, std::int64_t & value_out,
                const std::shared_ptr<coin_control> & control = 0
            ) const;

            /**
             * Creates a transaction.
             * @param scripts The scripts.
             * @param tx_new The transaction_wallet.
             * @param reserved_key The key_reserved.
             * @param fee_out The fee.
             * @param control The coin_control.
             */
            bool create_transaction(
                const std::vector< std::pair<script, std::int64_t> > & scripts,
                transaction_wallet & tx_new, key_reserved & reserved_key,
                std::int64_t & fee_out,
                const std::shared_ptr<coin_control> & control = 0
            );
      
            /**
             * Creates a transaction.
             * @param script_pub_key The script.
             * @param value The value.
             * @param tx_new The transaction_wallet.
             * @param reserved_key The key_reserved.
             * @param fee_out The fee (out).
             * @param control The coin_control.
             */
            bool create_transaction(
                const script & script_pub_key, const std::int64_t & value,
                transaction_wallet & tx_new, key_reserved & reserved_key,
                std::int64_t & fee_out,
                const std::shared_ptr<coin_control> & control = 0
            );
        
            /**
             * Commits a transaction.
             * @param wtx_new The transaction_wallet.
             * @param reserve_key The key_reserved.
             */
            bool commit_transaction(
                transaction_wallet & wtx_new, key_reserved & reserve_key
            );
        
            /**
             * Creates coin-stake.
             * @param keystore The key_store.
             * @param bits The bits.
             * @param search_interval The search interval.
             * @param tx_new The new transaction.
             */
            bool create_coin_stake(
                const key_store & keystore, const std::uint32_t & bits,
                const std::int64_t search_interval, transaction & tx_new
            );
        
            /**
             * Sends money.
             * @param script_pub_key The script public key.
             * @param value The value.
             * @param wtx_new The new transaction_wallet.
             */
            bool send_money(
                const script & script_pub_key, const std::int64_t & value,
                const transaction_wallet & wtx_new
            );
        
            /**
             * Sends money to the destination.
             * @param address The destination::tx_t.
             * @param value The value.
             * @param wtx_new The new transaction_wallet.
             */
            bool send_money_to_destination(
                const destination::tx_t & address, const std::int64_t & value,
                const transaction_wallet & wtx_new
            );
        
            /**
             * Populates coins with spendable outputs.
             * @param coins The output's.
             * @param only_confirmed If true only confirmed outputs will
             * be returned.
             * @param control The coin_control.
             */
            void available_coins(
                std::vector<output> & coins, const bool & only_confirmed,
                const std::shared_ptr<coin_control> & control
            ) const;

            /**
             * Selects coins min conf.
             * @param target_value The target time.
             * @param spend_time The spend time.
             * @param conf_mine The conf mine.
             * @param conf_theirs The conf theirs.
             * @param coins The coins (in).
             * @param coins_out The coins (out).
             * @param value_out The value (out).
             */
            bool select_coins_min_conf(
                std::int64_t target_value, std::uint32_t spend_time,
                std::int32_t conf_mine, std::int32_t conf_theirs,
                std::vector<output> coins,
                std::set< std::pair<transaction_wallet,
                std::uint32_t> > & coins_out, std::int64_t & value_out
            ) const;
        
            /**
             * Calculates the best approximate subset.
             * @param value The value.
             * @param total_lower The total lower.
             * @param target_value The target value.
             * @param bests The bests
             * @param best The best.
             * @param iterations The iterations.
             */
            static void approximate_best_subset(
                std::vector< std::pair< std::int64_t,
                std::pair<transaction_wallet, std::uint32_t> > > value,
                const std::int64_t & total_lower,
                const std::int64_t & target_value, std::vector<char> & bests,
                std::int64_t & best, const std::uint32_t & iterations = 1000
            );
        
            /**
             * The transactions.
             */
            std::map<sha256, transaction_wallet> & transactions();
        
            /**
             * The transactions.
             */
            const std::map<sha256, transaction_wallet> & transactions() const;
        
            /**
             * The request counts.
             */
            std::map<sha256, std::int32_t> & request_counts();
        
            /**
             * The address book.
             */
            std::map<destination::tx_t, std::string> & address_book();
        
            /**
             * The address book.
             */
            const std::map<destination::tx_t, std::string> & address_book() const;
        
            /**
             * Sets the order position next.
             * @param val The value.
             */
            void set_order_position_next(const std::int64_t & val);
        
            /**
             * The next order position.
             */
            const std::int64_t & order_position_next() const;
        
            /** 
             * Reads an order position.
             * @param order_position The order position.
             * @param value The value.
             */
            static void read_order_position(
                std::int64_t & order_position,
                std::map<std::string, std::string> & value
            );

            /**
             * Writes an order position.
             * @param order_position The order position.
             * @param value The value.
             */
            static void write_order_position(
                const std::int64_t & order_position,
                std::map<std::string, std::string> & value
            );
        
            /**
             * Gets an account balance given wallet database and name.
             * @param wallet_db The db_wallet.
             * @param account_name The account name.
             * @param minimum_depth The minimum depth in the blockchain.
             */
            static std::int64_t get_account_balance(
                db_wallet & wallet_db, const std::string & account_name,
                const std::size_t & minimum_depth
            );

            /**
             * Gets an account balance given name.
             * @param account_name The account name.
             * @param minimum_depth The minimum depth in the blockchain.
             */
            static std::int64_t get_account_balance(
                const std::string & account_name,
                const std::size_t & minimum_depth
            );
        
        private:

            /**
             * Checks the wallet integrity and performs repairs if necessary.
             * @param ec The boost::system::error_code.
             */
            void check_tick(const boost::system::error_code & ec);
        
            /**
             * Resends any transactions that have not yet made it into a block.
             * @param ec The boost::system::error_code.
             */
            void resend_transactions_tick(const boost::system::error_code & ec);
        
            /**
             * Encrypts the wallet.
             * @param passphrase The passphrase.
             * @note You MUST restart the process after calling this function.
             */
            bool do_encrypt(const std::string & passphrase);
        
            /**
             * The database wallet encryption.
             */
            std::shared_ptr<db_wallet> m_db_wallet_encryption;
        
            /**
             * The current wallet version. Clients below this version are not
             * able to load the wallet.
             */
            std::int32_t m_wallet_version;

            /**
             * The maximum wallet format version. The version this wallet may
             * be upgraded to.
             */
            std::int32_t m_wallet_version_max;
    
            /**
             * The transactions.
             */
            mutable std::map<sha256, transaction_wallet> m_transactions;
        
            /**
             * The request counts.
             */
            mutable std::map<sha256, std::int32_t> m_request_counts;
        
            /**
             * The address book.
             */
            mutable std::map<destination::tx_t, std::string> m_address_book;
        
            /**
             * The next order position.
             */
            mutable std::int64_t m_order_position_next;
        
            /**
             * The default public key.
             */
            mutable key_public m_key_public_default;
        
            /**
             * The key pool.
             */
            std::set<std::int64_t> m_key_pool;
        
            /**
             * The master keys.
             */
            std::map<std::uint32_t, key_wallet_master> m_master_keys;
        
            /**
             * The master key max id.
             */
            std::uint32_t m_master_key_max_id;
    
        protected:
        
            /**
             * The stack_impl.
             */
            stack_impl & stack_impl_;
            
            /**
             * The mutex.
             */
            mutable std::recursive_mutex mutex_;
        
            /**
             * If true the wallet is file backed.
             */
            bool is_file_backed_;
        
            /**
             * The check interval.
             */
            enum { interval_check = 86400 };
        
            /**
             * The check timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > check_timer_;
        
            /**
             * The resend transactions timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > resend_transactions_timer_;
        
            /**
             * The time of the last resend operation.
             */
            std::time_t time_last_resend_;
    };
    
} // namespace coin

#endif // COIN_WALLET_HPP