/*!
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This file is part of mini_analyser.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "minivideo_utils_qt.h"

// QCustomPlot widgets
#include "thirdparty/qcustomplot/qcustomplot.h"

#include <QDateTime>
#include <QVector>

#include <cstdint>
#include <cmath>

/* ************************************************************************** */

void MainWindow::cleanData()
{
    //QString unknown = tr("Unknown");
    //QString unknown_bold = tr("<b>Unknown</b>");

    // Infos tab
    ui->label_info_fullpath->clear();
    ui->label_info_filename->clear();
    ui->label_info_filesize->clear();
    ui->label_info_duration->clear();

    ui->label_info_container->clear();
    ui->label_info_container_profile->clear();
    ui->label_info_container_overhead->clear();
    ui->label_info_creation_date->clear();
    ui->label_info_creation_app->clear();
    ui->label_info_creation_lib->clear();

    ui->label_info_video_codec->clear();
    ui->label_info_video_duration->clear();
    ui->label_info_video_size->clear();
    ui->label_info_video_definition->clear();
    ui->label_info_video_aspectratio->clear();
    ui->label_info_video_display_definition->clear();
    ui->label_info_video_display_aspectratio->clear();
    ui->label_info_video_framerate->clear();
    ui->label_info_video_framerate_mode->clear();
    ui->label_info_video_bitrate->clear();
    ui->label_info_video_bitratemode->clear();

    ui->label_info_audio_codec->clear();
    ui->label_info_audio_duration->clear();
    ui->label_info_audio_size->clear();
    ui->label_info_audio_channels->clear();
    ui->label_info_audio_lng->clear();
    ui->label_info_audio_channelmode->clear();
    ui->label_info_audio_samplingrate->clear();
    ui->label_info_audio_bitpersample->clear();
    ui->label_info_audio_bitrate->clear();
    ui->label_info_audio_bitratemode->clear();

    // Infos tab // Other tracks
    if (ui->verticalLayout_other->layout())
    {
        QLayoutItem *item;
        while ((item = ui->verticalLayout_other->layout()->takeAt(0)))
        {
            delete item->widget();
            delete item;
        }
    }

    // Audio tab
    ui->comboBox_audio_selector->clear();
    ui->label_audio_id->clear();
    ui->label_audio_size->clear();
    ui->label_audio_duration->clear();
    ui->label_audio_title->clear();
    ui->label_audio_lng->clear();
    ui->label_audio_default->clear();
    ui->label_audio_forced->clear();

    ui->label_audio_fcc->clear();
    ui->label_audio_codec->clear();
    ui->label_audio_codec_profile->clear();
    ui->label_audio_encoder->clear();

    ui->label_audio_bitrate_gross->clear();
    ui->label_audio_bitrate_lowest->clear();
    ui->label_audio_bitrate_highest->clear();
    ui->label_audio_bitratemode->clear();
    ui->label_audio_compression_ratio->clear();

    ui->label_audio_channels->clear();
    ui->label_audio_channelmode->clear();
    ui->label_audio_samplingrate->clear();
    ui->label_audio_bitpersample->clear();
    ui->label_audio_sampleduration->clear();
    ui->label_audio_samplecount->clear();
    ui->label_audio_sampleperframe->clear();
    ui->label_audio_framecount->clear();
    ui->label_audio_frameduration->clear();

    // Video tab
    ui->comboBox_video_selector->clear();
    ui->label_video_id->clear();
    ui->label_video_size->clear();
    ui->label_video_duration->clear();
    ui->label_video_title->clear();
    ui->label_video_lng->clear();

    ui->label_video_fcc->clear();
    ui->label_video_codec->clear();
    ui->label_video_codec_profile->clear();
    ui->label_video_codec_infos->clear();
    ui->label_video_encoder->clear();

    ui->label_video_definition->clear();
    ui->label_video_sar->clear();
    ui->label_video_buffersize->clear();
    ui->label_video_definition_display->clear();
    ui->label_video_dar->clear();
    ui->label_video_par->clear();
    ui->label_video_rotation->clear();
    ui->label_video_projection->clear();
    ui->label_video_stereomode->clear();

    ui->label_video_framerate->clear();
    ui->label_video_framerate_mode->clear();
    ui->label_video_frameduration->clear();

    ui->label_video_bitrate_gross->clear();
    ui->label_video_bitrate_lowest->clear();
    ui->label_video_bitrate_highest->clear();
    ui->label_video_bitratemode->clear();
    ui->label_video_compression_ratio->clear();

    ui->label_video_color_space->clear();
    ui->label_video_color_subsampling->clear();
    ui->label_video_color_depth->clear();
    ui->label_video_color_range->clear();
    ui->label_video_color_primaries->clear();
    ui->label_video_color_transfer->clear();
    ui->label_video_color_matrix->clear();

    ui->label_video_framecount->clear();
    ui->label_video_samplecount->clear();
    ui->label_video_samplerepart->clear();

    // Subtitles tab
    ui->comboBox_sub_selector->clear();
    ui->label_sub_id->clear();
    ui->label_sub_size->clear();
    ui->label_sub_codec->clear();
    ui->label_sub_title->clear();
    ui->label_sub_lng->clear();
    ui->label_sub_encoding->clear();
    ui->textBrowser_sub->clear();

    // Others tab
    if (ui->verticalLayout_other2_track->layout())
    {
        QLayoutItem *item;
        while ((item = ui->verticalLayout_other2_track->layout()->takeAt(0)))
        {
            delete item->widget();
            delete item;
        }
    }
    if (ui->verticalLayout_other2_chapters->layout())
    {
        QLayoutItem *item;
        while ((item = ui->verticalLayout_other2_chapters->layout()->takeAt(0)))
        {
            delete item->widget();
            delete item;
        }
    }
    if (ui->verticalLayout_other2_tags->layout())
    {
        QLayoutItem *item;
        while ((item = ui->verticalLayout_other2_tags->layout()->takeAt(0)))
        {
            delete item->widget();
            delete item;
        }
    }
}

int MainWindow::setActiveFile()
{
    int status = 1;

    MediaFile_t *media = currentMediaFile();
    MediaWrapper *wrapper = currentMediaWrapper();

    if (media && wrapper)
    {
        // Don't reload the data if the file was already selected
        if (currentMediaLoaded != media->file_path)
        {
            // Set the file in the UI
            {
                wrapper->start_ui = std::chrono::steady_clock::now();

                handleTabWidget();

                cleanData();
                printData();
                ui->tab_container->loadMedia(wrapper);
                ui->tab_export->loadMedia(wrapper);

                currentMediaLoaded = media->file_path;

                wrapper->end_ui = std::chrono::steady_clock::now();

                resizeEvent(nullptr);
            }

#ifdef QT_DEBUG
            // Add the file to the dev tab
            {
                int64_t tp = std::chrono::duration_cast<std::chrono::milliseconds>(wrapper->end_parsing - wrapper->start_parsing).count();
                int64_t tt = std::chrono::duration_cast<std::chrono::milliseconds>(wrapper->end_ui - wrapper->start_ui).count();

                QString file = QString::fromUtf8(media->file_path);
                QString name = QString::fromUtf8(media->file_name);
                if (strlen(media->file_extension) > 0)
                    name += "." + QString::fromUtf8(media->file_extension);

                ui->tab_dev->addFile(file, name, tt, tp, media->parsingMemory);
            }
#endif // QT_DEBUG
        }

        status = 0;
    }

    return status;
}

/* ************************************************************************** */

int MainWindow::printData()
{
    int status = 1;

    MediaFile_t *media = currentMediaFile();
    MediaWrapper *wrapper = currentMediaWrapper();

    if (media && wrapper)
    {
        // Combobox icon
        if (media->tracks_video_count > 0)
            ui->comboBox_file->setItemIcon(ui->comboBox_file->currentIndex(), icon_movie);
        else if (media->tracks_audio_count > 0)
            ui->comboBox_file->setItemIcon(ui->comboBox_file->currentIndex(), icon_music);
        else
            ui->comboBox_file->setItemIcon(ui->comboBox_file->currentIndex(), icon_error);

        // General file infos
        QString name = QString::fromUtf8(media->file_name);
        if (strlen(media->file_extension) > 0)
            name += "." + QString::fromUtf8(media->file_extension);

        ui->label_info_filename->setText(name);
        ui->label_info_filename->setToolTip(QString::fromUtf8(media->file_name));
        ui->label_info_fullpath->setText(QString::fromUtf8(media->file_path));
        ui->label_info_fullpath->setToolTip(QString::fromUtf8(media->file_path));
        ui->label_info_filesize->setText(getSizeQString(media->file_size));

        ui->label_3->setVisible(media->duration);
        ui->label_info_duration->setVisible(media->duration);
        ui->label_info_duration->setText(getDurationQString(media->duration));

        // Container infos
        ui->label_info_container->setText(getContainerString(media->container, true));

        ui->label_83->setVisible(media->container_profile);
        ui->label_info_container_profile->setVisible(media->container_profile);
        ui->label_info_container_profile->setText(getContainerProfileString(media->container_profile, true));

        // Container extension mismatch?
        ui->label_info_container_mismatch->setVisible(media->container_sc != media->container_ext);
        ui->label_info_container_mismatch->setText(
            QString(tr("File extension (%1) does not match container (%2)"))
                    .arg(getContainerString(media->container_ext, false))
                    .arg(getContainerString(media->container_sc, false)));

        ui->label_3->setVisible(media->duration);
        ui->label_info_duration->setVisible(media->duration);
        ui->label_info_duration->setText(getDurationQString(media->duration));

        ui->label_ca->setVisible(media->creation_app);
        ui->label_info_creation_app->setVisible(media->creation_app);
        ui->label_info_creation_app->setText(QString::fromUtf8(media->creation_app));

        ui->label_cl->setVisible(media->creation_lib);
        ui->label_info_creation_lib->setVisible(media->creation_lib);
        ui->label_info_creation_lib->setText(QString::fromUtf8(media->creation_lib));

        if (media->creation_time)
        {
            ui->label_cd->setVisible(true);
            ui->label_info_creation_date->setVisible(true);

            QDate date(1970, 1, 1);
            QTime time(0, 0, 0, 0);
            QDateTime datetime(date, time);

            datetime = datetime.addSecs(media->creation_time);
            //datetime = datetime.toUTC();
            ui->label_info_creation_date->setText(datetime.toString("dddd d MMMM yyyy, hh:mm:ss"));
        }
        else
        {
            ui->label_cd->setVisible(false);
            ui->label_info_creation_date->setVisible(false);
        }

        // Container efficiency
        if (media->tracks_audio_count > 0 || media->tracks_video_count > 0 ||
            media->tracks_subtitles_count > 0 || media->tracks_others_count > 0)
        {
            ui->label_8->setVisible(true);
            ui->label_info_container_overhead->setVisible(true);

            int64_t trackssize = 0;
            for (int i = 0; i < 16; i++)
            {
                if (media->tracks_audio[i])
                    trackssize += media->tracks_audio[i]->stream_size;
                if (media->tracks_video[i])
                    trackssize += media->tracks_video[i]->stream_size;
                if (media->tracks_subt[i])
                    trackssize += media->tracks_subt[i]->stream_size;
                if (media->tracks_others[i])
                    trackssize += media->tracks_others[i]->stream_size;
            }

            int64_t overhead = media->file_size - trackssize;
            double overheadpercent = (static_cast<double>(overhead) / static_cast<double>(media->file_size)) * 100.0;

            if (overheadpercent < 0.01)
                ui->label_info_container_overhead->setText("<b>~0.01%</b>   >   " + getSizeQString(overhead));
            else if (overheadpercent <= 100)
                ui->label_info_container_overhead->setText("<b>" + QString::number(overheadpercent, 'f', 2) + "%</b>   >   " + getSizeQString(overhead));
            else
                ui->label_info_container_overhead->setText("<b>(ERR)</b>");
        }
        else
        {
            ui->label_8->setVisible(false);
            ui->label_info_container_overhead->setVisible(false);
        }

        // AUDIO
        ////////////////////////////////////////////////////////////////////////

        if (media->tracks_audio_count == 0 || media->tracks_audio[0] == nullptr)
        {
            ui->groupBox_infos_audio->hide();
        }
        else
        {
            ui->groupBox_infos_audio->show();

            const MediaStream_t *t = media->tracks_audio[0];

            ui->label_info_audio_size->setText(getTrackSizeQString(t, media->file_size));
            ui->label_info_audio_codec->setText(getCodecQString(stream_AUDIO, t->stream_codec, true));
            ui->label_info_audio_duration->setText(getDurationQString(t->stream_duration_ms));

            QString lng = getLanguageQString(t->track_languagecode);
            if (lng.isEmpty())
            {
                ui->label_24->hide();
                ui->label_info_audio_lng->hide();
            }
            else
            {
                ui->label_24->show();
                ui->label_info_audio_lng->show();
                ui->label_info_audio_lng->setText(lng);
            }

            ui->label_86->setVisible(t->channel_mode);
            ui->label_info_audio_channelmode->setVisible(t->channel_mode);
            ui->label_info_audio_channelmode->setText(getChannelModeQString(t->channel_mode));

            ui->label_info_audio_bitrate->setText(getBitrateQString(t->bitrate_avg));
            ui->label_info_audio_samplingrate->setText(QString::number(t->sampling_rate) + " Hz");
            ui->label_info_audio_channels->setText(QString::number(t->channel_count));

            if (t->bit_per_sample)
            {
                ui->label_info_audio_bitpersample->setText(QString::number(t->bit_per_sample) + " bits");
            }
            ui->label_info_audio_bitratemode->setText(getBitrateModeQString(t->bitrate_mode));

            if (media->tracks_audio_count > 1)
            {
                ui->comboBox_audio_selector->clear();
                ui->comboBox_audio_selector->show();

                for (unsigned i = 0; i < media->tracks_audio_count; i++)
                {
                    if (media->tracks_audio[i])
                    {
                        QString text = "Audio track #" + QString::number(i + 1);
                        if (i != media->tracks_audio[i]->track_id)
                            text += " (internal id #" + QString::number(media->tracks_audio[i]->track_id) + ")";
                        QString track_language = getLanguageQString(media->tracks_audio[i]->track_languagecode);
                        if (!track_language.isEmpty())
                            text += " [" + track_language + "]";
                        ui->comboBox_audio_selector->addItem(text);
                    }
                }

                ui->comboBox_audio_selector->setCurrentIndex(wrapper->currentAudioTrack);
            }
            else
            {
                ui->comboBox_audio_selector->hide();
            }

            printAudioDetails();
        }

        // VIDEO
        ////////////////////////////////////////////////////////////////////////

        if (media->tracks_video_count == 0 || media->tracks_video[0] == nullptr)
        {
            ui->groupBox_infos_video->hide();
        }
        else
        {
            ui->groupBox_infos_video->show();

            const MediaStream_t *t = media->tracks_video[0];

            ui->label_info_video_codec->setText(getCodecQString(stream_VIDEO, t->stream_codec, true));
            ui->label_info_video_duration->setText(getDurationQString(t->stream_duration_ms));
            ui->label_info_video_bitrate->setText(getBitrateQString(t->bitrate_avg));
            ui->label_info_video_definition->setText(QString::number(t->width) + " x " + QString::number(t->height));
            ui->label_info_video_aspectratio->setText(getAspectRatioQString(t->video_aspect_ratio));

            ui->label_93->setVisible(t->width_display != t->width || t->height_display != t->height);
            ui->label_info_video_display_definition->setVisible(t->width_display != t->width || t->height_display != t->height);
            ui->label_info_video_display_definition->setText(QString::number(t->width_display) + " x " + QString::number(t->height_display));
            ui->label_95->setVisible(t->display_aspect_ratio != t->video_aspect_ratio);
            ui->label_info_video_display_aspectratio->setVisible(t->display_aspect_ratio != t->video_aspect_ratio);
            ui->label_info_video_display_aspectratio->setText(getAspectRatioQString(t->display_aspect_ratio));

            ui->label_info_video_framerate->setText(QString::number(t->framerate) + " FPS");
            ui->label_info_video_size->setText(getTrackSizeQString(t, media->file_size));
            ui->label_info_video_bitratemode->setText(getBitrateModeQString(t->bitrate_mode));

            ui->label_7->setVisible(t->framerate_mode);
            ui->label_info_video_framerate_mode->setVisible(t->framerate_mode);
            ui->label_info_video_framerate_mode->setText(getFramerateModeQString(t->framerate_mode));

            ui->label_84->setVisible(t->video_projection);
            ui->label_info_video_projection->setVisible(t->video_projection);
            ui->label_info_video_projection->setText(getProjectionQString(t->video_projection));

            ui->label_88->setVisible(t->video_rotation);
            ui->label_info_video_rotation->setVisible(t->video_rotation);
            ui->label_info_video_rotation->setText(getRotationQString(t->video_rotation));

            if (media->tracks_video_count > 1)
            {
                ui->comboBox_video_selector->show();
                for (unsigned i = 0; i < media->tracks_video_count; i++)
                {
                    if (media->tracks_video[i])
                    {
                        QString text = "Video track #" + QString::number(i + 1);
                        if (i != media->tracks_video[i]->track_id)
                            text += " (internal id #" + QString::number(media->tracks_video[i]->track_id) + ")";
                        ui->comboBox_video_selector->addItem(text);
                    }
                }
            }
            else
            {
                ui->comboBox_video_selector->hide();
            }

            printVideoDetails();
        }

        // SUBS
        ////////////////////////////////////////////////////////////////////////

        if (media->tracks_subtitles_count > 0)
        {
            if (media->tracks_subtitles_count > 1)
            {
                ui->comboBox_sub_selector->show();
                for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
                {
                    if (media->tracks_subt[i])
                    {
                        QString text = "Subtitles track #" + QString::number(i + 1);
                        if (i != media->tracks_subt[i]->track_id)
                            text += " (internal id #" + QString::number(media->tracks_subt[i]->track_id) + ")";
                        QString track_language = getLanguageQString(media->tracks_subt[i]->track_languagecode);
                        if (!track_language.isEmpty())
                            text += " [" + track_language + "]";
                        ui->comboBox_sub_selector->addItem(text);
                    }
                }
            }
            else
            {
                ui->comboBox_sub_selector->hide();
            }

            printSubtitlesDetails();
        }

        // Other track(s)
        ////////////////////////////////////////////////////////////////////////

        if (media->tracks_audio_count <= 1 &&
            media->tracks_video_count <= 1 &&
            media->tracks_subtitles_count <= 0 &&
            media->tracks_others_count <= 0)
        {
            ui->groupBox_infos_other->hide();
        }
        else
        {
            ui->groupBox_infos_other->show();

            // Add new tracks
            for (unsigned i = 1; i < media->tracks_video_count; i++)
            {
                if (media->tracks_video[i])
                {
                    QString text = "▸ " + tr("Video track #") + QString::number(i+1) + "  /  ";
                    text += getCodecQString(stream_VIDEO, media->tracks_video[i]->stream_codec, false);

                    QLabel *track = new QLabel(text);
                    ui->verticalLayout_other->addWidget(track);
                }
            }
            for (unsigned i = 1; i < media->tracks_audio_count; i++)
            {
                if (media->tracks_audio[i])
                {
                    QString text = "▸ " + tr("Audio track #") + QString::number(i+1) + "  /  ";
                    text += getCodecQString(stream_AUDIO, media->tracks_audio[i]->stream_codec, false);
                    QString lng = getLanguageQString(media->tracks_audio[i]->track_languagecode);
                    if (lng.isEmpty() == false)
                    {
                        text += "  /  ";
                        text += lng;
                    }

                    QLabel *track = new QLabel(text);
                    ui->verticalLayout_other->addWidget(track);
                }
            }
            for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
            {
                if (media->tracks_subt[i])
                {
                    QString text = "▸ " + tr("Subtitles track #") + QString::number(i+1);

                    if (media->tracks_subt[i]->stream_codec)
                    {
                        text += "  /  ";
                        text += getCodecQString(stream_TEXT, media->tracks_subt[i]->stream_codec, false);
                    }

                    QString lng = getLanguageQString(media->tracks_subt[i]->track_languagecode);
                    if (lng.isEmpty() == false)
                    {
                        text += "  /  ";
                        text += lng;
                    }

                    QString title = QString::fromUtf8(media->tracks_subt[i]->track_title);
                    if (title.isEmpty() == false)
                    {
                        text += "  /  ";
                        text += title;
                    }

                    QLabel *track = new QLabel(text);

                    ui->verticalLayout_other->addWidget(track);
                }
            }
            for (unsigned i = 0; i < media->tracks_others_count; i++)
            {
                if (media->tracks_others[i])
                {
                    const MediaStream_t *t = media->tracks_others[i];

                    QString text = "▸ " + getTrackTypeQString(t) + tr(" track (internal id  #") + QString::number(t->track_id) + ")";

                    if (t->stream_type == stream_TMCD)
                    {
                        text += QString("\n   SMPTE TimeCode '%1:%2:%3-%4'")\
                                .arg(t->time_reference[0], 2, 'u', 0, '0')\
                                .arg(t->time_reference[1], 2, 'u', 0, '0')\
                                .arg(t->time_reference[2], 2, 'u', 0, '0')\
                                .arg(t->time_reference[3], 2, 'u', 0, '0');
                    }

                    QLabel *track = new QLabel(text);
                    ui->verticalLayout_other->addWidget(track);
                }
            }

            printOtherDetails();
        }

        status = 0;
    }

    return status;
}

/* ************************************************************************** */

int MainWindow::printAudioDetails()
{
    int status = 1;

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        int atid = ui->comboBox_audio_selector->currentIndex();
        if (atid < 0) atid = 0;

        MediaStream_t *t = media->tracks_audio[atid];

        if (t)
        {
            ui->groupBox_tab_audio->setTitle(tr("Audio track") + " #" + QString::number(atid + 1));

            ui->label_audio_id->setText(QString::number(t->track_id));

            QString track_title = QString::fromUtf8(t->track_title);
            if (track_title.isEmpty())
            {
                ui->label_64->hide();
                ui->label_audio_title->hide();
            }
            else
            {
                ui->label_64->show();
                ui->label_audio_title->show();
                ui->label_audio_title->setText(track_title);
            }

            QString stream_encoder = QString::fromUtf8(t->stream_encoder);
            if (stream_encoder.isEmpty())
            {
                ui->label_61->hide();
                ui->label_audio_encoder->hide();
            }
            else
            {
                ui->label_61->show();
                ui->label_audio_encoder->show();
                ui->label_audio_encoder->setText(stream_encoder);
            }

            QString track_language = getLanguageQString(t->track_languagecode);
            if (track_language.isEmpty())
            {
                ui->label_26->hide();
                ui->label_audio_lng->hide();
            }
            else
            {
                ui->label_26->show();
                ui->label_audio_lng->show();
                ui->label_audio_lng->setText(track_language);
            }

            if (t->track_default || t->track_forced)
            {
                ui->label_31->show();
                ui->label_audio_default->show();
                ui->label_32->show();
                ui->label_audio_forced->show();

                if (t->track_default)
                    ui->label_audio_default->setText("✔");
                if (t->track_forced)
                    ui->label_audio_forced->setText("✔");
            }
            else
            {
                ui->label_31->hide();
                ui->label_audio_default->hide();
                ui->label_32->hide();
                ui->label_audio_forced->hide();
            }

            ui->label_audio_size->setText(getTrackSizeQString(t, media->file_size, true));
            ui->label_audio_codec->setText(getCodecQString(stream_AUDIO, t->stream_codec, true));

            if (t->stream_codec_profile)
            {
                ui->label_82->setVisible(true);
                ui->label_audio_codec_profile->setVisible(true);
                ui->label_audio_codec_profile->setText(getCodecProfileString(t->stream_codec_profile, true));
            }
            else
            {
                ui->label_82->setVisible(false);
                ui->label_audio_codec_profile->setVisible(false);
            }

            if (t->stream_fcc)
            {
                ui->label_12->setVisible(true);
                ui->label_audio_fcc->setVisible(true);

                char fcc_str[5];
                ui->label_audio_fcc->setText(QString::fromLatin1(getFccString_le(t->stream_fcc, fcc_str), 4));
            }
            else
            {
                ui->label_12->setVisible(false);
                ui->label_audio_fcc->setVisible(false);
            }

            if (t->channel_mode)
            {
                ui->label_6->show();
                ui->label_audio_channelmode->show();
                ui->label_audio_channelmode->setText(getChannelModeQString(t->channel_mode));
            }
            else
            {
                ui->label_6->hide();
                ui->label_audio_channelmode->hide();
            }

            ui->label_audio_duration->setText(getDurationQString(t->stream_duration_ms));

            ui->label_audio_bitrate_gross->setText(getBitrateQString(t->bitrate_avg));
            ui->label_audio_bitratemode->setText(getBitrateModeQString(t->bitrate_mode));

            ui->label_audio_samplingrate->setText(QString::number(t->sampling_rate) + " Hz");
            ui->label_audio_channels->setText(QString::number(t->channel_count));

            if (t->bit_per_sample)
            {
                ui->label_audio_bitpersample->setText(QString::number(t->bit_per_sample) + " bits");
            }

            if (t->stream_size)
            {
                uint64_t rawsize = t->sampling_rate * t->channel_count * (t->bit_per_sample / 8);
                rawsize *= t->stream_duration_ms;
                rawsize /= 1000;

                uint64_t ratio = std::round(static_cast<double>(rawsize) / static_cast<double>(t->stream_size));
                ui->label_audio_compression_ratio->setText(QString::number(ratio) + ":1");
            }

            ui->label_audio_samplecount->setText(QString::number(t->sample_count));
            ui->label_audio_framecount->setText(QString::number(t->frame_count));

            if (t->sample_duration > 0.0)
                ui->label_audio_sampleduration->setText(QString::number(t->sample_duration) + " µs");

            if (t->sample_per_frames > 0)
                ui->label_audio_sampleperframe->setText(QString::number(t->sample_per_frames));

            if (t->frame_duration < 1.0)
                ui->label_audio_frameduration->setText(QString::number(t->frame_duration*1000000.0) + " µs");
            else
                ui->label_audio_frameduration->setText(QString::number(t->frame_duration) + " ms");

            // Audio bitrate graph /////////////////////////////////////////////

            if (t->bitrate_mode == BITRATE_CBR)
            {
                ui->audioBitrateGraph->hide();
                ui->label_73->hide();
                ui->label_audio_bitrate_highest->hide();
                ui->label_74->hide();
                ui->label_audio_bitrate_lowest->hide();
            }
            else
            {
                ui->audioBitrateGraph->show();
                ui->label_73->show();
                ui->label_audio_bitrate_highest->show();
                ui->label_74->show();
                ui->label_audio_bitrate_lowest->show();

                double fps = 0;
                if (t->stream_duration_ms > 0) fps = (static_cast<double>(t->sample_count) / (static_cast<double>(t->stream_duration_ms)/1000.0));
                bitrateMinMax btc(fps);
                uint32_t bitratemin = 0, bitratemax = 0, bitratemax_instant = 0;

                // Generate data (bitrate) from A/V samples
                uint32_t entries = t->sample_count;
                QVector<double> x(entries), y(entries);
                for (uint32_t i = 0; i < entries; i++)
                {
                    if (t->sample_type[i] == sample_AUDIO)
                    {
                        x[i] = static_cast<double>(i);
                        y[i] = static_cast<double>(t->sample_size[i]);

                        if (t->sample_size[i] > bitratemax_instant)
                            bitratemax_instant = t->sample_size[i];

                        btc.pushSampleSize(t->sample_size[i]);
                    }
                }

                // Generate data (average bitrate)
                QVector<double> xx(2), yy(2);
                xx[0] = 0;
                xx[1] = entries;
                yy[0] = yy[1] = static_cast<double>(t->bitrate_avg) / 8.0 / (static_cast<double>(entries) / (t->stream_duration_ms/1000.0));

                // MinMax
                btc.getMinMax(bitratemin, bitratemax);
                t->bitrate_min = bitratemin;
                t->bitrate_max = bitratemax;
                ui->label_audio_bitrate_lowest->setText(getBitrateQString(bitratemin));
                ui->label_audio_bitrate_highest->setText(getBitrateQString(bitratemax));

                // Create graphs and assign data
                ui->audioBitrateGraph->addGraph();
                ui->audioBitrateGraph->setBackground(QBrush(QColor(0, 0, 0, 0)));
                ui->audioBitrateGraph->graph(0)->setData(x, y);
                ui->audioBitrateGraph->graph(0)->setBrush(QBrush(QColor(10, 10, 200, 20)));
                ui->audioBitrateGraph->addGraph();
                ui->audioBitrateGraph->graph(1)->setData(xx, yy);
                ui->audioBitrateGraph->graph(1)->setPen(QPen(QColor(220, 60, 0, 240)));

                // Set axes ranges and disable legends
                ui->audioBitrateGraph->xAxis->setRange(0, entries);
                ui->audioBitrateGraph->xAxis->setTickLabels(false);
                ui->audioBitrateGraph->yAxis->setRange(0, bitratemax_instant);
                ui->audioBitrateGraph->yAxis->setTickLabels(false);

                //ui->audioBitrateGraph->setContentsMargins(0,0,0,0);
                //ui->audioBitrateGraph->yAxis->setPadding(0);
                //ui->audioBitrateGraph->xAxis->setPadding(0);
                //ui->audioBitrateGraph->setBackground(QColor(0,0,0,0));

#ifdef Q_OS_MACOS
                //ui->audioBitrateGraph->setBackground(QColor(218, 218, 218));
#endif

                // Allow interactions
                //ui->audioBitrateGraph->setInteraction(QCP::iRangeDrag, true);
                //ui->audioBitrateGraph->setInteraction(QCP::iRangeZoom, true);
                //connect(ui->audioBitrateGraph->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisRangeChanged(QCPRange)));
                //connect(ui->audioBitrateGraph->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisRangeChanged(QCPRange)));

                ui->audioBitrateGraph->replot();
            }
        }

        status = 0;
    }

    return status;
}

/* ************************************************************************** */

int MainWindow::printVideoDetails()
{
    int status = 1;

    MediaFile_t *media = currentMediaFile();
    MediaWrapper *wrapper = currentMediaWrapper();

    if (media && wrapper)
    {
        int vtid = ui->comboBox_video_selector->currentIndex();
        if (vtid < 0) vtid = 0;

        MediaStream_t *t = media->tracks_video[vtid];

        if (t)
        {
            ui->groupBox_tab_video->setTitle(tr("Video track") + " #" + QString::number(vtid + 1));

            ui->label_video_id->setText(QString::number(t->track_id));

            QString track_title = QString::fromUtf8(t->track_title);
            if (track_title.isEmpty())
            {
                ui->label_63->hide();
                ui->label_video_title->hide();
            }
            else
            {
                ui->label_63->show();
                ui->label_video_title->show();
                ui->label_video_title->setText(track_title);
            }

            QString stream_encoder = QString::fromUtf8(t->stream_encoder);
            if (stream_encoder.isEmpty())
            {
                ui->label_62->hide();
                ui->label_video_encoder->hide();
            }
            else
            {
                ui->label_62->show();
                ui->label_video_encoder->show();
                ui->label_video_encoder->setText(stream_encoder);
            }

            QString track_language = getLanguageQString(t->track_languagecode);
            if (track_language.isEmpty())
            {
                ui->label_33->hide();
                ui->label_video_lng->hide();
            }
            else
            {
                ui->label_33->show();
                ui->label_video_lng->show();
                ui->label_video_lng->setText(track_language);
            }

            ui->label_video_size->setText(getTrackSizeQString(t, media->file_size, true));
            ui->label_video_codec->setText(getCodecQString(stream_VIDEO, t->stream_codec, true));

            if (t->stream_codec_profile)
            {
                QString str = "<b>" + QString::fromUtf8(getCodecProfileString(t->stream_codec_profile, true)) + "</b>";
                if (t->video_level > 0) str += " @ <b>L" + QString::number(t->video_level, 'g', 2) + "</b>";

                ui->label_71->setVisible(true);
                ui->label_video_codec_profile->setVisible(true);
                ui->label_video_codec_profile->setText(str);
            }
            else
            {
                ui->label_71->setVisible(false);
                ui->label_video_codec_profile->setVisible(false);
            }

            if (t->h264_feature_cabac || t->h264_feature_8x8 || t->max_ref_frames > 0)
            {
                QString str;
                if (t->stream_codec == CODEC_H264)
                {
                    if (t->h264_feature_cabac) str = "CABAC";
                    else str = "CAVLC";
                }
                if (t->h264_feature_8x8 == true)
                {
                    if (!str.isEmpty()) str += " / ";
                    str += tr("8x8 blocks");
                }
                if (t->max_ref_frames > 0)
                {
                    if (!str.isEmpty()) str += " / ";
                    str += QString::number(t->max_ref_frames) + " " + tr("reference frames");
                }

                ui->label_90->setVisible(true);
                ui->label_video_codec_infos->setVisible(true);
                ui->label_video_codec_infos->setText(str);
            }
            else
            {
                ui->label_90->setVisible(false);
                ui->label_video_codec_infos->setVisible(false);
            }

            ui->label_14->setVisible(t->stream_fcc);
            ui->label_video_fcc->setVisible(t->stream_fcc);

            if (t->stream_fcc)
            {
                char fcc_str[5];
                ui->label_video_fcc->setText(QString::fromLatin1(getFccString_le(t->stream_fcc, fcc_str), 4));
            }

            ui->label_video_duration->setText(getDurationQString(t->stream_duration_ms));

            ui->label_video_bitrate_gross->setText(getBitrateQString(t->bitrate_avg));
            ui->label_video_bitratemode->setText(getBitrateModeQString(t->bitrate_mode));

            ui->label_video_definition->setText(QString::number(t->width) + " x " + QString::number(t->height));
            ui->label_video_sar->setText(getAspectRatioQString(t->video_aspect_ratio, true));

            if (t->video_projection)
            {
                ui->label_5->setVisible(true);
                ui->label_video_projection->setVisible(true);
                ui->label_video_projection->setText(getProjectionQString(t->video_projection));
            }
            else
            {
                ui->label_5->setVisible(false);
                ui->label_video_projection->setVisible(false);
            }
            if (t->video_rotation)
            {
                ui->label_87->setVisible(true);
                ui->label_video_rotation->setVisible(true);
                ui->label_video_rotation->setText(getRotationQString(t->video_rotation));
            }
            else
            {
                ui->label_87->setVisible(false);
                ui->label_video_rotation->setVisible(false);
            }
            if (t->stereo_mode)
            {
                ui->label_85->setVisible(true);
                ui->label_video_stereomode->setVisible(true);
                ui->label_video_stereomode->setText(getStereoModeQString(t->stereo_mode));
            }
            else
            {
                ui->label_85->setVisible(false);
                ui->label_video_stereomode->setVisible(false);
            }
            if ((t->width_encoded > 0 && t->width_encoded != t->width) ||
                (t->height_encoded > 0 &&t->height_encoded != t->height))
            {
                ui->label_96->setVisible(true);
                ui->label_video_buffersize->setVisible(true);
                ui->label_video_buffersize->setText(QString::number(t->width_encoded) + " x " + QString::number(t->height_encoded));
            }
            else
            {
                ui->label_96->setVisible(false);
                ui->label_video_buffersize->setVisible(false);
            }
            if (t->video_aspect_ratio != t->display_aspect_ratio)
            {
                ui->label_10->setVisible(true);
                ui->label_video_definition_display->setVisible(true);
                ui->label_video_definition_display->setText(QString::number(t->width_display) + " x " + QString::number(t->height_display));
                ui->label_81->setVisible(true);
                ui->label_video_dar->setVisible(true);
                ui->label_video_dar->setText(getAspectRatioQString(t->display_aspect_ratio, false));
            }
            else
            {
                ui->label_10->setVisible(false);
                ui->label_video_definition_display->setVisible(false);
                ui->label_81->setVisible(false);
                ui->label_video_dar->setVisible(false);
            }
            if (t->pixel_aspect_ratio != 1.0)
            {
                ui->label_38->setVisible(true);
                ui->label_video_par->setVisible(true);
                ui->label_video_par->setText(getAspectRatioQString(t->pixel_aspect_ratio, false));
            }
            else
            {
                ui->label_38->setVisible(false);
                ui->label_video_par->setVisible(false);
            }

            if (t->color_space > 0 && t->color_subsampling > 0)
            {
                ui->label_50->setVisible(true);
                ui->label_60->setVisible(true);
                ui->label_video_color_space->setVisible(true);
                ui->label_video_color_subsampling->setVisible(true);

                if (t->color_space == CLR_RGB)
                    ui->label_video_color_space->setText("RGB");
                else if (t->color_space == CLR_xvYCC)
                    ui->label_video_color_space->setText("xvYCC");
                else if (t->color_space == CLR_YPbPr)
                    ui->label_video_color_space->setText("YPbPr");
                else if (t->color_space == CLR_YCbCr)
                    ui->label_video_color_space->setText("YCbCr");
                else if (t->color_space == CLR_YCgCo)
                    ui->label_video_color_space->setText("YCgCo");
                else if (t->color_space == CLR_ICtCp)
                    ui->label_video_color_space->setText("YICtCp");
                else
                    ui->label_video_color_space->setText("YCbCr (best guess)");

                if (t->color_subsampling == SS_4444)
                    ui->label_video_color_subsampling->setText("4:4:4:4");
                else if (t->color_subsampling == SS_444)
                    ui->label_video_color_subsampling->setText("4:4:4");
                else if (t->color_subsampling == SS_422)
                    ui->label_video_color_subsampling->setText("4:2:2");
                else if (t->color_subsampling == SS_420)
                    ui->label_video_color_subsampling->setText("4:2:0");
                else if (t->color_subsampling == SS_411)
                    ui->label_video_color_subsampling->setText("4:1:1");
                else if (t->color_subsampling == SS_400)
                    ui->label_video_color_subsampling->setText("4:0:0");
                else
                    ui->label_video_color_subsampling->setText("4:2:0 (best guess)");
            }
            else
            {
                ui->label_50->setVisible(false);
                ui->label_60->setVisible(false);
                ui->label_video_color_space->setVisible(false);
                ui->label_video_color_subsampling->setVisible(false);
            }

            if (t->color_depth > 0)
            {
                ui->label_58->setVisible(true);
                ui->label_89->setVisible(true);
                ui->label_video_color_range->setVisible(true);
                ui->label_video_color_depth->setVisible(true);

                ui->label_video_color_depth->setText(QString::number(t->color_depth) + " bits");

                if (t->color_range == 1)
                    ui->label_video_color_range->setText(tr("Full range"));
                else
                    ui->label_video_color_range->setText(tr("Limited range"));
            }
            else
            {
                ui->label_58->setVisible(false);
                ui->label_89->setVisible(false);
                ui->label_video_color_range->setVisible(false);
                ui->label_video_color_depth->setVisible(false);
            }

            if (t->color_primaries && t->color_transfer)
            {
                ui->label_65->setVisible(true);
                ui->label_92->setVisible(true);
                ui->label_94->setVisible(true);
                ui->label_video_color_primaries->setVisible(true);
                ui->label_video_color_transfer->setVisible(true);
                ui->label_video_color_matrix->setVisible(true);

                QString prim = getColorPrimariesString((ColorPrimaries_e)t->color_primaries);
                if (prim.isEmpty()) prim = "Rec. 709 " + tr("(best guess)");
                ui->label_video_color_primaries->setText(prim);

                QString tra = getColorTransferCharacteristicString((ColorTransferCharacteristic_e)t->color_transfer);
                if (tra.isEmpty()) tra = "Rec. 709 " + tr("(best guess)");
                ui->label_video_color_transfer->setText(tra);

                QString mat = getColorMatrixString((ColorSpace_e)t->color_matrix);
                if (mat.isEmpty()) mat = "Rec. 709 " + tr("(best guess)");
                ui->label_video_color_matrix->setText(mat);
            }
            else
            {
                ui->label_65->setVisible(false);
                ui->label_92->setVisible(false);
                ui->label_94->setVisible(false);
                ui->label_video_color_primaries->setVisible(false);
                ui->label_video_color_transfer->setVisible(false);
                ui->label_video_color_matrix->setVisible(false);
            }

            double framerate = t->framerate;
            if (framerate < 1.0)
            {
                if (t->stream_duration_ms && t->sample_count)
                {
                    framerate = static_cast<double>(t->sample_count / (static_cast<double>(t->stream_duration_ms) / 1000.0));
                }
            }
            double frameduration = 0.0;
            if (framerate > 0.0)
            {
                frameduration = 1000.0 / framerate; // in ms
            }

            QString samplecount = "<b>" + QString::number(t->sample_count) + "</b>";
            QString framecount = "<b>" + QString::number(t->frame_count) + "</b>";
            QString samplerepartition;

            if (t->frame_count_idr)
            {
                framecount += "      (" + QString::number(t->frame_count_idr) + " IDR  /  ";
                framecount += QString::number(t->frame_count - t->frame_count_idr) + " others)";

                double idr_ratio = static_cast<double>(t->frame_count_idr) / static_cast<double>(t->sample_count) * 100.0;
                QString idr_string = QString::number(idr_ratio, 'g', 2);
                if (t->frame_count_idr == t->sample_count) idr_string = QString::number(idr_ratio);

                samplerepartition = tr("IDR frames makes <b>") + idr_string + "%</b> " + tr("of the samples");
                samplerepartition += "<br>Statistically, one every <b>";
                samplerepartition += QString::number(t->stream_duration_ms / 1000.0 / static_cast<double>(t->frame_count_idr), 'g', 2);
                samplerepartition += "</b> s";

                ui->label_video_samplerepart->setText(samplerepartition);
            }

            ui->label_video_samplecount->setText(samplecount);
            ui->label_video_framecount->setText(framecount);

            // Framerate
            ui->label_video_framerate->setText(QString::number(framerate) + " FPS");

            // Framerate mode
            ui->label_77->setVisible(t->framerate_mode);
            ui->label_video_framerate_mode->setVisible(t->framerate_mode);
            ui->label_video_framerate_mode->setText(getFramerateModeQString(t->framerate_mode));

            // Frame duration
                ui->label_34->setVisible(frameduration > 0);
                ui->label_video_frameduration->setVisible(frameduration > 0);
                ui->label_video_frameduration->setText(QString::number(frameduration, 'g', 4) + " ms");

            // Compression ratio
            if (t->stream_size)
            {
                if (t->color_planes == 0) t->color_planes = 3;
                if (t->color_depth == 0) t->color_depth = 8;

                uint64_t rawsize = t->width * t->height * (t->color_depth / 8) * t->color_planes;
                rawsize *= t->sample_count;

                uint64_t ratio = std::round(static_cast<double>(rawsize) / static_cast<double>(t->stream_size));
                ui->label_video_compression_ratio->setText(QString::number(ratio) + ":1");
            }

            // Video bitrate graph /////////////////////////////////////////////

            if (t->bitrate_mode == BITRATE_CBR)
            {
                ui->videoBitrateGraph->hide();
                ui->label_56->hide();
                ui->label_video_bitrate_highest->hide();
                ui->label_72->hide();
                ui->label_video_bitrate_lowest->hide();
            }
            else
            {
                ui->videoBitrateGraph->show();
                ui->label_56->show();
                ui->label_video_bitrate_highest->show();
                ui->label_72->show();
                ui->label_video_bitrate_lowest->show();

                bitrateMinMax btc(t->framerate);
                uint32_t bitratemin = 0, bitratemax = 0, bitratemax_instant = 0;

                // Generate data (bitrate) from A/V samples
                uint32_t entries = t->sample_count;
                QVector<double> x(entries), y(entries);
                for (uint32_t i = 0; i < entries; i++)
                {
                    if (t->sample_type[i] == sample_VIDEO || t->sample_type[i] == sample_VIDEO_SYNC)
                    {
                        x[i] = static_cast<double>(i);
                        y[i] = static_cast<double>(t->sample_size[i]);

                        if (t->sample_size[i] > bitratemax_instant)
                            bitratemax_instant = t->sample_size[i];

                        btc.pushSampleSize(t->sample_size[i]);
                    }
                }

                // Generate data (average bitrate)
                QVector<double> xx(2), yy(2);
                xx[0] = 0;
                xx[1] = entries;
                yy[0] = yy[1] = static_cast<double>(t->bitrate_avg) / 8.0 / framerate;

                // MinMax
                btc.getMinMax(bitratemin, bitratemax);
                t->bitrate_min = bitratemin;
                t->bitrate_max = bitratemax;

                ui->label_video_bitrate_lowest->setText(getBitrateQString(bitratemin));
                ui->label_video_bitrate_highest->setText(getBitrateQString(bitratemax));
                wrapper->xRangeMax = static_cast<double>(entries);
                wrapper->yRangeMax = static_cast<double>(bitratemax_instant);

                // Create graphs and assign data
                ui->videoBitrateGraph->addGraph();
                ui->videoBitrateGraph->setBackground(QBrush(QColor(0, 0, 0, 0)));
                ui->videoBitrateGraph->graph(0)->setData(x, y);
                ui->videoBitrateGraph->graph(0)->setBrush(QBrush(QColor(10, 10, 200, 20)));
                ui->videoBitrateGraph->addGraph();
                ui->videoBitrateGraph->graph(1)->setData(xx, yy);
                ui->videoBitrateGraph->graph(1)->setPen(QPen(QColor(220, 60, 0, 240)));

                // Y axis max is set between 50% and 100% of the max instant bitrate (helps reduce the video IDR spikes)
                double yScale = 1.0;
                if (yy[0] < wrapper->yRangeMax / 10)
                    yScale = 0.5;
                else if (yy[0] < wrapper->yRangeMax / 2)
                    yScale = 0.75;

                // Set axes ranges and disable legends
                ui->videoBitrateGraph->xAxis->setRange(0, wrapper->xRangeMax);
                ui->videoBitrateGraph->xAxis->setTickLabels(false);
                ui->videoBitrateGraph->yAxis->setRange(0, wrapper->yRangeMax * yScale);
                ui->videoBitrateGraph->yAxis->setTickLabels(false);

                //ui->videoBitrateGraph->setContentsMargins(0,0,0,0);
                //ui->videoBitrateGraph->yAxis->setPadding(0);
                //ui->videoBitrateGraph->xAxis->setPadding(0);
                //ui->videoBitrateGraph->setBackground(QColor(0,0,0,0));

#ifdef Q_OS_MACOS
                //ui->videoBitrateGraph->setBackground(QColor(218, 218, 218));
#endif

                // Allow interactions
                //ui->videoBitrateGraph->setInteraction(QCP::iRangeDrag, true);
                //ui->videoBitrateGraph->setInteraction(QCP::iRangeZoom, true);
                //connect(ui->videoBitrateGraph->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisRangeChanged(QCPRange)));
                //connect(ui->videoBitrateGraph->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisRangeChanged(QCPRange)));

                ui->videoBitrateGraph->replot();
            }
        }

        status = 0;
    }

    return status;
}

/* ************************************************************************** */

int MainWindow::printSubtitlesDetails()
{
    int status = 1;

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        int subid = ui->comboBox_sub_selector->currentIndex();
        if (subid < 0) subid = 0;

        MediaStream_t *t = media->tracks_subt[subid];

        if (t)
        {
            ui->groupBox_tab_subtitles->setTitle(tr("Subtitles track") + " #" + QString::number(subid + 1));

            ui->label_sub_id->setText(QString::number(t->track_id));

            QString track_title = QString::fromUtf8(t->track_title);
            if (track_title.isEmpty())
            {
                ui->label_67->hide();
                ui->label_sub_title->hide();
            }
            else
            {
                ui->label_67->show();
                ui->label_sub_title->show();
                ui->label_sub_title->setText(track_title);
            }

            QString track_language = getLanguageQString(t->track_languagecode);
            if (track_language.isEmpty())
            {
                ui->label_66->hide();
                ui->label_sub_lng->hide();
            }
            else
            {
                ui->label_66->show();
                ui->label_sub_lng->show();
                ui->label_sub_lng->setText(track_language);
            }

            ui->label_sub_size->setText(getTrackSizeQString(t, media->file_size, true));
            ui->label_sub_codec->setText(getCodecQString(stream_TEXT, t->stream_codec, true));

            // TODO // set subtitles encoding!
            ui->label_80->setVisible(false);
            ui->label_sub_encoding->setVisible(false);
            //ui->label_sub_encoding->setText("<b>?</b>");

            QString text;

            if (t->stream_codec == CODEC_SRT ||
                t->stream_codec == CODEC_ASS ||
                t->stream_codec == CODEC_MPEG4_TTXT)
            {
                for (unsigned i = 0; i < t->sample_count; i++)
                {
                    text += "[" + getTimestampPreciseQString(t->sample_pts[i]) + "]\n";

                    MediaSample_t *s = minivideo_get_sample(media, t, i);
                    if (s)
                    {
                        if (t->stream_codec == CODEC_SRT)
                            text += QString::fromUtf8((const char *)(s->data), s->size);

                        if (t->stream_codec == CODEC_ASS) // WIP
                            text += QString::fromUtf8((const char *)(s->data), s->size);

                        if (t->stream_codec == CODEC_MPEG4_TTXT) // WIP
                            text += QString::fromUtf8((const char *)(s->data + 2), s->size - 2);

                        minivideo_destroy_sample(&s);
                        text += "\n\n";
                    }
                }
            }

            ui->textBrowser_sub->setText(text);
            //if (text.isEmpty())
            //    ui->textBrowser_sub->hide();
            //else
            //    ui->textBrowser_sub->show();
        }

        status = 0;
    }

    return status;
}

/* ************************************************************************** */

int MainWindow::printOtherDetails()
{
    int status = 1;

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        // Add new tracks
        for (unsigned i = 0; i < media->tracks_others_count; i++)
        {
            if (media->tracks_others[i])
            { // ▸
                const MediaStream_t *t = media->tracks_others[i];

                QString text = "<b>" + getTrackTypeQString(t) + tr(" track</b> (internal id  #") + QString::number(t->track_id) + ")";

                if (t->track_title)
                {
                    text += tr("<br>- Title: ");
                    text += t->track_title;
                }

                if (media->tracks_others[i]->stream_type == stream_TMCD &&
                    media->tracks_others[i]->sample_count == 1)
                {
                    text += QString("<br>- SMPTE TimeCode '%1:%2:%3-%4'")\
                            .arg(t->time_reference[0], 2, 'u', 0, '0')\
                            .arg(t->time_reference[1], 2, 'u', 0, '0')\
                            .arg(t->time_reference[2], 2, 'u', 0, '0')\
                            .arg(t->time_reference[3], 2, 'u', 0, '0');
                }

                text += tr("<br>- Size: ");
                text += getSizeQString(t->stream_size);

                text += tr("<br>- ");
                text += QString::number(t->sample_count);
                text += tr(" samples");

                if (i < media->tracks_others_count-1) text += tr("<br>");

                QLabel *track = new QLabel(text);
                ui->verticalLayout_other2_track->addWidget(track);
            }
        }

        // Add new chapters
        if (media->chapters_count > 0 && media->chapters)
        {
            QLabel *chap = new QLabel(tr("%1 chapters").arg(media->chapters_count));
            ui->verticalLayout_other2_chapters->addWidget(chap);

            QListWidget *chl = new QListWidget();
            for (unsigned i = 0; i < media->chapters_count; i++)
            {
                Chapter_t *ch = &media->chapters[i];
                if (ch == nullptr) break;

                QString chstr = "#" + QString::number(i);
                if (ch->name) chstr += " '" + QString::fromUtf8(ch->name) + "'";
                chstr += " @ " + QString::number(ch->pts / 1000.f) + "s";

                new QListWidgetItem(chstr, chl);
            }
            ui->verticalLayout_other2_chapters->addWidget(chl);
        }
        else
        {
            QLabel *chap = new QLabel(tr("No chapters found..."));
            ui->verticalLayout_other2_chapters->addWidget(chap);
        }

        // Add new tags
        QLabel *tag = new QLabel(tr("No tags found..."));
        ui->verticalLayout_other2_tags->addWidget(tag);

        status = 0;
    }

    return status;
}

/* ************************************************************************** */

void MainWindow::xAxisRangeChanged(const QCPRange &newRange)
{
    Q_UNUSED(newRange)

    // TODO
}

void MainWindow::yAxisRangeChanged(const QCPRange &newRange)
{
    Q_UNUSED(newRange)

    // TODO
}

/* ************************************************************************** */
