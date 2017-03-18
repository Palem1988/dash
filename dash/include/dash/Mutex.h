#ifndef DASH__MUTEX_H__INCLUDED
#define DASH__MUTEX_H__INCLUDED

#include <dash/Exception.h>
#include <dash/internal/TypeInfo.h>

#include <cstdint>
#include <iostream>

#include "Team.h"


namespace dash {

/**
 * Behaves similar to \cstd::mutex and is used to ensure mutual exclusion
 * within a dash team.
 * 
 * \note This works properly with \cstd::lock_guard
 * 
 * \code
 * // just for demonstration, better use atomic operations
 * dash::Mutex mx; // mutex for dash::Team::All();
 * dash::Array<int> arr(10);
 * {
 *    std::lock_guard<dash::Mutex> lg(mx);
 *    int tmp = arr[0];
 *    arr[0] = tmp + 1;
 *    // TODO: this almost certainly requires a flush
 * }
 * dash::barrier();
 * // postcondition: arr[0] == dash::size();
 * \endcode
 */
class Mutex {
private:
  using self_t = Mutex;

public:
  /**
   * DASH Mutex is only valid for a dash team. If no team is passed, team all
   * is used.
   * 
   * This function is not thread-safe
   * @param team team for mutual exclusive accesses
   */
  explicit Mutex(Team & team = dash::Team::All()):
    _team(team)
  {
    dart_ret_t ret = dart_team_lock_init(_team.dart_id(), &_mutex);
    DASH_ASSERT_EQ(DART_OK, ret, "dart_team_lock_init failed");
  }
  
  Mutex(const Mutex & other)               = delete;
  Mutex(Mutex && other)                    = default;

  self_t & operator=(const self_t & other) = delete;
  
  /**
   * Collective destructor to destruct a DART lock.
   * 
   * This function is not thread-safe
   */
  ~Mutex(){
    dart_ret_t ret = dart_team_lock_free(_team.dart_id(), &_mutex);
    DASH_ASSERT_EQ(DART_OK, ret, "dart_team_lock_free failed");
  }
  
  /**
   * Block until the lock was acquired.
   */
  void lock(){
    dart_ret_t ret = dart_lock_acquire(_mutex);
    DASH_ASSERT_EQ(DART_OK, ret, "dart_lock_acquire failed");
  }
  
  /**
   * Try to acquire the lock and return immediately.
   * @return True if lock was successfully aquired, False otherwise
   */
  bool try_lock(){
    int32_t result;
    dart_ret_t ret = dart_lock_try_acquire(_mutex, &result);
    DASH_ASSERT_EQ(DART_OK, ret, "dart_lock_try_acquire failed");
    return static_cast<bool>(result);
  }
  
  /**
   * Release the lock acquired through \clock() or \ctry_lock().
   */
  void unlock(){
    dart_ret_t ret = dart_lock_release(_mutex);
    DASH_ASSERT_EQ(DART_OK, ret, "dart_lock_acquire failed");
  }

private:
  dart_lock_t   _mutex;
  Team        & _team;
}; // class Atomic

} // namespace dash

#endif // DASH__MUTEX_H__INCLUDED

