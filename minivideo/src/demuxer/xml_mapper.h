/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
 *
 * This file is part of MiniVideo.
 *
 * MiniVideo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MiniVideo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MiniVideo.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \file      xml_mapper.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#ifndef XML_MAPPER_H
#define XML_MAPPER_H

// minivideo headers
#include "../import.h"

/* ************************************************************************** */

/*!
 * \brief Open the xmlMapper file and write header content.
 * \param *media[in]: A pointer to a MediaFile_t structure.
 * \param **xml[in,out]: Mapping file.
 */
int xmlMapperOpen(MediaFile_t *media, FILE **xml);

/*!
 * \brief Finalize the xmlMapper file and write footer content.
 * \param *xml[in]: Mapping file.
 */
int xmlMapperFinalize(FILE *xml);

/*!
 * \brief Close the xmlMapper.
 * \param **xml[in,out]: Mapping file.
 */
int xmlMapperClose(FILE **xml);

/* ************************************************************************** */
/*
// xmlMapper format v1

<?xml version="1.0"?>
<file xmlMapper="0.1" minivideo="0.7-2">
<header>
  <format>WAVE</format>
  <size>3211820</size>
  <path>/path/to/this/file.wav</path>
</header>
<structure>
  <atom fcc="WAVE" type="RIFF header" offset="0" size="3211812">
    <atom fcc="fmt " type="RIFF chunk" offset="12" size="16">
          <wFormatTag>1</wFormatTag>
          <nChannels>4</nChannels>
          <nSamplesPerSec>48000</nSamplesPerSec>
          <nAvgBytesPerSec note="unreliable">384000</nAvgBytesPerSec>
          <nBlockAlign>8</nBlockAlign>
          <wBitsPerSample unit="bit">16</wBitsPerSample>
    </atom>
    <atom fcc="data" type="RIFF chunk" offset="36" size="3211776">
      <datasOffset>44</datasOffset>
      <datasSize>3211776</datasSize>
    </atom>
  </atom>
</structure>
</file>
*/
/* ************************************************************************** */
#endif // XML_MAPPER_H
