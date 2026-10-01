#pragma once

// Drops the settings an older NomIME left in the user folder so that a newly
// installed version starts from its own defaults instead of inheriting patches
// written against the previous one.
//
// The user's own data -- typed words (*.userdb), dictionary snapshots and the
// sync folder -- is never touched.
//
// The settings on disk are tagged with the version that wrote them, so this is
// a no-op once the current version has been through it. Returns true when a
// reset actually happened.
bool ResetSettingsOnVersionChange();
