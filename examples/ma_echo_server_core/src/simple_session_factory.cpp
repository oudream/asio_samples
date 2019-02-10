//
// Copyright (c) 2010-2015 Marat Abrarov (abrarov@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <new>
#include <ma/shared_ptr_factory.hpp>
#include <ma/echo/server/error.hpp>
#include <ma/echo/server/simple_session_factory.hpp>
#include <ma/detail/memory.hpp>
#include <ma/detail/functional.hpp>

namespace ma {
namespace echo {
namespace server {

class simple_session_factory::session_wrapper
  : public session_wrapper_base
  , public session
{
private:
  typedef session_wrapper this_type;

public:
  static session_wrapper_ptr create(boost::asio::io_service& io_service,
      const session_config& config)
  {
    typedef shared_ptr_factory_helper<this_type> helper;
    return detail::make_shared<helper>(detail::ref(io_service), config);
  }

protected:
  session_wrapper(boost::asio::io_service& io_service,
      const session_config& config)
    : session(io_service, config)
  {
  }

  ~session_wrapper()
  {
  }
}; // class simple_session_factory::session_wrapper

session_ptr simple_session_factory::create(const session_config& config,
    boost::system::error_code& error)
{
  if (!recycled_.empty())
  {
    session_wrapper_ptr session =
        detail::static_pointer_cast<session_wrapper>(recycled_.front());
    recycled_.erase(session);
    error = boost::system::error_code();
    return session;
  }

  try
  {
    session_wrapper_ptr session = session_wrapper::create(io_service_, config);
    error = boost::system::error_code();
    return session;
  }
  catch (const std::bad_alloc&)
  {
    error = boost::system::errc::make_error_code(
        boost::system::errc::not_enough_memory);
    return session_ptr();
  }
}

void simple_session_factory::release(const session_ptr& session)
{
  if (max_recycled_ > recycled_.size())
  {
    recycled_.push_front(detail::static_pointer_cast<session_wrapper>(session));
  }
}

} // namespace server
} // namespace echo
} // namespace ma
