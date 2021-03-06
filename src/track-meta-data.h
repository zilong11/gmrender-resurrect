// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/* track-meta-data - Object holding meta data for a song.
 *
 * Copyright (C) 2012 Henner Zeller
 *
 * This file is part of GMediaRender.
 *
 * GMediaRender is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GMediaRender is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GMediaRender; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
#ifndef _TRACK_META_DATA_H
#define _TRACK_META_DATA_H

#include <gst/gsttaglist.h>

#include <string>
#include <functional>
#include <unordered_map>

// Metadata for a song that can be filled by GStreamer tags and import/export
// as DIDL-Lite XML, used in the UPnP world to describe track metadata.
class TrackMetadata {
public:
  typedef const char *TagLabel; // Opaque type used in labels.
  static TagLabel TAG_TITLE;
  static TagLabel TAG_ARTIST;
  static TagLabel TAG_ALBUM;
  static TagLabel TAG_GENRE;
  static TagLabel TAG_COMPOSER;

  // Get field with given tag.
  const std::string& get_field(TagLabel tag) const;

  // Set the field with the given "tag" to "value". Returns 'true' if
  // the field value changed.
  bool set_field(TagLabel tag, const std::string &value);

  // Convenience getter for title.
  const std::string& title() const { return get_field(TAG_TITLE); }

  //-- there are more fields that can be added when needed.

  void Clear() { fields_.clear(); }

  // Update from GstTags. Return if there was any change.
  bool UpdateFromTags(const GstTagList *tag_list);

  // Returns xml string with the song meta data encoded as
  // DIDL-Lite. If we get a non-empty original xml document, returns an
  // edited version of that document.
  // "idgen" is a generator for the toplevel identifier attribute of the
  // document; if null, a default generator is used.
  std::string ToXML(const std::string &original_xml,
                    std::function<std::string()> idgen = nullptr) const;

  // Parse DIDL-Lite and fill TrackMetaData. Returns true when successful.
  bool ParseXML(const std::string &xml);

private:
  typedef std::unordered_map<std::string, std::string> MetaMap;

  static std::string DefaultCreateNewId();

  // Generate a new DIDL XML.
  std::string GenerateDIDL(const std::string &id) const;

  // We store the fields keyed by their XML name, as this is the primary
  // interaction with the outside world.
  MetaMap fields_;
};

#endif  // _TRACK_META_DATA_H
