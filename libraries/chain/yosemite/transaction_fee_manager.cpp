/**
 *  @file chain/yosemite/transaction_fee_manager.cpp
 *  @author bezalel@yosemitex.com
 *  @copyright defined in yosemite/LICENSE.txt
 */

#include <yosemite/chain/transaction_fee_manager.hpp>
#include <yosemite/chain/exceptions.hpp>

#include <eosio/chain/database_utils.hpp>

namespace yosemite { namespace chain {

   using namespace eosio::chain;

   using transaction_fee_index_set = index_set<
      transaction_fee_multi_index
   >;

   transaction_fee_manager::transaction_fee_manager(chainbase::database& db)
      : _db(db) {
   }

   void transaction_fee_manager::add_indices() {
      transaction_fee_index_set::add_indices(_db);
   }

   void transaction_fee_manager::initialize_database() {

   }

   void transaction_fee_manager::add_to_snapshot( const snapshot_writer_ptr& snapshot ) const {
      transaction_fee_index_set::walk_indices([this, &snapshot]( auto utils ){
         snapshot->write_section<typename decltype(utils)::index_t::value_type>([this]( auto& section ){
            decltype(utils)::walk(_db, [this, &section]( const auto &row ) {
               section.add_row(row, _db);
            });
         });
      });
   }

   void transaction_fee_manager::read_from_snapshot( const snapshot_reader_ptr& snapshot ) {
      transaction_fee_index_set::walk_indices([this, &snapshot]( auto utils ){
         snapshot->read_section<typename decltype(utils)::index_t::value_type>([this]( auto& section ) {
            bool more = !section.empty();
            while(more) {
               decltype(utils)::create(_db, [this, &section, &more]( auto &row ) {
                  more = section.read_row(row, _db);
               });
            }
         });
      });
   }

   void transaction_fee_manager::set_tx_fee_for_action(
      const account_name& code, const action_name& action, const tx_fee_value_type value, const tx_fee_type_type fee_type ) {

      //EOS_ASSERT( !code.empty() && !action.empty(), invalid_tx_fee_setup_exception, "set_transaction_fee - code and action must be not 0" );
      EOS_ASSERT( value >= 0, yosemite_transaction_fee_exception,  "tx fee value must be >= 0" );
      EOS_ASSERT( fee_type == fixed_tx_fee_per_action_type, yosemite_transaction_fee_exception,
                  "currently set_tx_fee_for_action supports only fixed_tx_fee_per_action_type" );

      auto* txfee_obj_ptr = _db.find<transaction_fee_object, by_code_action>(boost::make_tuple(code, action));
      if ( txfee_obj_ptr ) {
         _db.modify( *txfee_obj_ptr, [&](transaction_fee_object& txfee_obj) {
            txfee_obj.value = value;
            txfee_obj.fee_type = fee_type;
         });
      } else {
         _db.create<transaction_fee_object>( [&](transaction_fee_object& txfee_obj) {
            txfee_obj.code = code;
            txfee_obj.action = action;
            txfee_obj.value = value;
            txfee_obj.fee_type = fee_type;
         });
      }
   }

   void transaction_fee_manager::set_tx_fee_for_common_action(const action_name& action, const tx_fee_value_type value, const tx_fee_type_type fee_type) {
      set_tx_fee_for_action(0, action, value, fee_type);
   }

   void transaction_fee_manager::set_default_tx_fee(const tx_fee_value_type value, const tx_fee_type_type fee_type) {
      set_tx_fee_for_action(0, 0, value, fee_type);
   }

   void transaction_fee_manager::delete_set_tx_fee_entry_for_action(const account_name& code, const action_name& action ) {
      auto* txfee_obj_ptr = _db.find<transaction_fee_object, by_code_action>(boost::make_tuple(code, action));
      EOS_ASSERT( txfee_obj_ptr, yosemite_transaction_fee_exception,  "tx fee db row not found" );
      _db.remove( *txfee_obj_ptr );
   }

   tx_fee_for_action transaction_fee_manager::get_tx_fee_for_action(const account_name& code, const action_name& action) const {

      auto* txfee_obj_ptr = _db.find<transaction_fee_object, by_code_action>(boost::make_tuple(code, action));
      if ( txfee_obj_ptr ) {
         auto txfee_obj = *txfee_obj_ptr;
         return tx_fee_for_action { txfee_obj.value, txfee_obj.fee_type };
      } else {
         return get_tx_fee_for_common_action(action);
      }
   }

   tx_fee_for_action transaction_fee_manager::get_tx_fee_for_common_action(const action_name& action) const {

      auto* txfee_obj_ptr = _db.find<transaction_fee_object, by_code_action>(boost::make_tuple(account_name(0), action));
      if ( txfee_obj_ptr ) {
         auto txfee_obj = *txfee_obj_ptr;
         return tx_fee_for_action { txfee_obj.value, txfee_obj.fee_type };
      } else {
         return get_default_tx_fee();
      }
   }

   tx_fee_for_action transaction_fee_manager::get_default_tx_fee() const {

      auto* txfee_obj_ptr = _db.find<transaction_fee_object, by_code_action>(boost::make_tuple(account_name(0), action_name(0)));
      if ( txfee_obj_ptr ) {
         auto txfee_obj = *txfee_obj_ptr;
         return tx_fee_for_action { txfee_obj.value, txfee_obj.fee_type };
      } else {
         return tx_fee_for_action { 100000, fixed_tx_fee_per_action_type };
      }
   }

   tx_fee_for_action transaction_fee_manager::get_tx_fee_for_action_trace(const action_trace& action_trace) const {
      const account_name& code = action_trace.receipt.receiver;
      if (code == action_trace.act.account) {  // exclude notified actions
         return get_tx_fee_for_action(code, action_trace.act.name);
      } else {
         return tx_fee_for_action { 0, fixed_tx_fee_per_action_type };
      }
   }

   tx_fee_list_result transaction_fee_manager::get_tx_fee_list(const account_name& code_lower_bound, const account_name& code_upper_bound, const uint32_t limit) const {

      tx_fee_list_result result;
      EOS_ASSERT( code_upper_bound.empty() || code_lower_bound <= code_upper_bound, yosemite_transaction_fee_exception,  "get_tx_fee_list requires code_lower_bound <= code_upper_bound" );
      const auto& idx = _db.get_index<transaction_fee_multi_index, by_code_action>();
      auto lower = idx.begin();
      if (code_lower_bound.good()) {
         lower = idx.lower_bound(boost::make_tuple(code_lower_bound));
      }
      auto upper = idx.end();
      if (code_upper_bound.good()) {
         upper = idx.upper_bound(boost::make_tuple(code_upper_bound));
      }

      auto time_limit = fc::time_point::now() + fc::microseconds(1000 * 50); /// 50ms max time

      unsigned int count = 0;
      auto itr = lower;
      for (; itr != upper; ++itr) {
         auto txfee_obj = (*itr);
         result.tx_fee_list.push_back(tx_fee_list_item{txfee_obj.code, txfee_obj.action, txfee_obj.value, txfee_obj.fee_type});

         if (++count == limit || fc::time_point::now() > time_limit) {
            ++itr;
            break;
         }
      }
      if (itr != upper) {
         result.more = true;
      }

      return result;
   }

} } // namespace yosemite::chain
