#pragma once

#include <eosio/chain/types.hpp>
#include <eosio/wallet_plugin/wallet_api.hpp>

using namespace std;
using namespace eosio::chain;

namespace eosio { namespace wallet {

namespace detail {
struct se_wallet_impl;
}

class se_wallet final : public wallet_api {
   public:
      se_wallet();
      ~se_wallet();

      private_key_type get_private_key(const public_key_type &pubkey) const override;

      bool is_locked() const override;
      void lock() override;
      void unlock(const string &password) override;
      void check_password(const string &password) override;
      void set_password(const string &password) override;

      map<public_key_type, private_key_type> list_keys() override;
      flat_set<public_key_type> list_public_keys() override;

      bool import_key(const string &wif_key) override;
      string create_key(const string &key_type) override;
      bool remove_key(const string &key) override;

      optional<signature_type> try_sign_digest(const digest_type &digest, const public_key_type &public_key) override;

   private:
      std::unique_ptr<detail::se_wallet_impl> my;
};

}}