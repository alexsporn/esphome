#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include <stdlib.h>
#include <time.h>
#include <bitset>

namespace esphome {
namespace time {

/// A more user-friendly version of struct tm from time.h
struct ESPTime {
  /** seconds after the minute [0-60]
   * @note second is generally 0-59; the extra range is to accommodate leap seconds.
   */
  uint8_t second;
  /// minutes after the hour [0-59]
  uint8_t minute;
  /// hours since midnight [0-23]
  uint8_t hour;
  /// day of the week; sunday=1 [1-7]
  uint8_t day_of_week;
  /// day of the month [1-31]
  uint8_t day_of_month;
  /// day of the year [1-366]
  uint16_t day_of_year;
  /// month; january=1 [1-12]
  uint8_t month;
  /// year
  uint16_t year;
  /// daylight savings time flag
  bool is_dst;
  union {
    ESPDEPRECATED(".time is deprecated, use .timestamp instead") time_t time;
    /// unix epoch time (seconds since UTC Midnight January 1, 1970)
    time_t timestamp;
  };

  /** Convert this ESPTime struct to a null-terminated c string buffer as specified by the format argument.
   * Up to buffer_len bytes are written.
   *
   * @see https://www.gnu.org/software/libc/manual/html_node/Formatting-Calendar-Time.html#index-strftime
   */
  size_t strftime(char *buffer, size_t buffer_len, const char *format);

  /** Convert this ESPTime struct to a string as specified by the format argument.
   * @see https://www.gnu.org/software/libc/manual/html_node/Formatting-Calendar-Time.html#index-strftime
   *
   * @warning This method uses dynamically allocated strings which can cause heap fragmentation with some
   * microcontrollers.
   */
  std::string strftime(const std::string &format);

  /// Check if this ESPTime is valid (all fields in range and year is greater than 2018)
  bool is_valid() const { return this->year >= 2019 && this->fields_in_range(); }

  /// Check if all time fields of this ESPTime are in range.
  bool fields_in_range() const {
    return this->second < 61 && this->minute < 60 && this->hour < 24 && this->day_of_week > 0 &&
           this->day_of_week < 8 && this->day_of_month > 0 && this->day_of_month < 32 && this->day_of_year > 0 &&
           this->day_of_year < 367 && this->month > 0 && this->month < 13;
  }

  /// Convert a C tm struct instance with a C unix epoch timestamp to an ESPTime instance.
  static ESPTime from_c_tm(struct tm *c_tm, time_t c_time);

  /** Convert an UTC epoch timestamp to a local time ESPTime instance.
   *
   * @param epoch Seconds since 1st January 1970. In UTC.
   * @return The generated ESPTime
   */
  static ESPTime from_epoch_local(time_t epoch) {
    struct tm *c_tm = ::localtime(&epoch);
    return ESPTime::from_c_tm(c_tm, epoch);
  }
  /** Convert an UTC epoch timestamp to a UTC time ESPTime instance.
   *
   * @param epoch Seconds since 1st January 1970. In UTC.
   * @return The generated ESPTime
   */
  static ESPTime from_epoch_utc(time_t epoch) {
    struct tm *c_tm = ::gmtime(&epoch);
    return ESPTime::from_c_tm(c_tm, epoch);
  }

  /// Recalculate the timestamp field from the other fields of this ESPTime instance (must be UTC).
  void recalc_timestamp_utc(bool use_day_of_year = true);

  /// Convert this ESPTime instance back to a tm struct.
  struct tm to_c_tm();

  /// Increment this clock instance by one second.
  void increment_second();
  bool operator<(ESPTime other);
  bool operator<=(ESPTime other);
  bool operator==(ESPTime other);
  bool operator>=(ESPTime other);
  bool operator>(ESPTime other);
};

/// The RealTimeClock class exposes common timekeeping functions via the device's local real-time clock.
///
/// \note
/// The C library (newlib) available on ESPs only supports TZ strings that specify an offset and DST info;
/// you cannot specify zone names or paths to zoneinfo files.
/// \see https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
class RealTimeClock : public Component {
 public:
  explicit RealTimeClock();

  /// Set the time zone.
  void set_timezone(const std::string &tz) { this->timezone_ = tz; }

  /// Get the time zone currently in use.
  std::string get_timezone() { return this->timezone_; }

  /// Get the time in the currently defined timezone.
  ESPTime now() { return ESPTime::from_epoch_local(this->timestamp_now()); }

  /// Get the time without any time zone or DST corrections.
  ESPTime utcnow() { return ESPTime::from_epoch_utc(this->timestamp_now()); }

  /// Get the current time as the UTC epoch since January 1st 1970.
  time_t timestamp_now() { return ::time(nullptr); }

  void call_setup() override;

 protected:
  /// Report a unix epoch as current time.
  void synchronize_epoch_(uint32_t epoch);

  std::string timezone_{};
};

}  // namespace time
}  // namespace esphome
