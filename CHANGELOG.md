# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]

## [1.4] - 2015-08-14
### Added
- Watchface configuration support.
- Customizable beats display font, with two additional fonts to choose from.

## [1.3] - 2015-07-21
### Changed
- Fixed calculations for UTC timezone offset and Internet Time; now the
  @beats time value should be correct for all timezones.

## [1.2] - 2015-06-19
### Changed
- Normal time display (HH:MM) uses now a slightly larger font.
- Normal time display respects the 24h/12h setting from the watch settings.
### Added
- The beats display has now a custom font.
- Bluetooth status is now tracked, and a short vibration produced on
  disconnect events.

## [1.1] - 2015-06-15
### Changed
- Fixed calculation of @beats time.
- On Basalt (Pebble Time), the HH:MM text display has light gray color.
- Background color is now black.

### Added
- Added small display the time in usual HH:MM format (24h format only
  for now, no AM/PM).

## 1.0 - 2015-06-12
### Added
- First release.

[Unreleased]: https://github.com/aperezdc/pebble-beats/compare/v1.4...HEAD
[1.4]: https://github.com/aperezdc/pebble-beats/compare/v1.3...v1.4
[1.3]: https://github.com/aperezdc/pebble-beats/compare/v1.2...v1.3
[1.2]: https://github.com/aperezdc/pebble-beats/compare/v1.1...v1.2
[1.1]: https://github.com/aperezdc/pebble-beats/compare/v1.0...v1.1
