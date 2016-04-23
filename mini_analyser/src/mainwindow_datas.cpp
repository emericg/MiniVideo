/*!
 * COPYRIGHT (C) 2016 Emeric Grange - All Rights Reserved
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
 * \file      mainwindow_datas.cpp
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2016
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QDateTime>

#include <iostream>
#include <cmath>

/* ************************************************************************** */

void MainWindow::cleanDatas()
{
    QString unknown = tr("Unknown");
    QString unknown_bold = tr("<b>Unknown</b>");

    // Infos tab
    ui->label_info_filename->clear();
    ui->label_info_fullpath->clear();
    ui->label_info_container->clear();
    ui->label_info_filesize->clear();
    ui->label_info_duration->clear();
    ui->label_info_creationapp->clear();
    ui->label_info_creationdate->clear();
    ui->label_info_container_overhead->clear();
    ui->label_info_audio_bitpersample->clear();
    ui->label_info_audio_bitrate->clear();
    ui->label_info_audio_bitratemode->clear();
    ui->label_info_audio_channels->clear();
    ui->label_info_audio_codec->clear();
    ui->label_info_audio_duration->clear();
    ui->label_info_audio_lng->clear();
    ui->label_info_audio_samplingrate->clear();
    ui->label_info_audio_size->clear();
    ui->label_info_video_dar->clear();
    ui->label_info_video_bitrate->clear();
    ui->label_info_video_bitratemode->clear();
    ui->label_info_video_codec->clear();
    ui->label_info_video_definition->clear();
    ui->label_info_video_duration->clear();
    ui->label_info_video_framerate->clear();
    ui->label_info_video_framerate_mode->clear();
    ui->label_info_video_size->clear();

    // Audio tab
    ui->comboBox_audio_selector->clear();
    ui->label_audio_default->clear();
    ui->label_audio_duration->clear();
    ui->label_audio_id->clear();
    ui->label_audio_lng->clear();
    ui->label_audio_size->clear();
    ui->label_audio_title->clear();
    ui->label_audio_visible->clear();

    ui->label_audio_bitratemode->clear();
    ui->label_audio_bitrate_gross->clear();
    ui->label_audio_bitrate_lowest->clear();
    ui->label_audio_bitrate_high->clear();
    ui->label_audio_codec->clear();
    ui->label_audio_compression_ratio->clear();
    ui->label_audio_encoder->clear();
    ui->label_audio_fcc->clear();

    ui->label_audio_bitpersample->clear();
    ui->label_audio_channels->clear();
    ui->label_audio_samplingrate->clear();

    ui->label_audio_framecount->clear();
    ui->label_audio_frameduration->clear();
    ui->label_audio_samplecount->clear();
    ui->label_audio_sampleperframe->clear();

    // Video tab
    ui->comboBox_video_selector->clear();
    ui->label_video_duration->clear();
    ui->label_video_id->clear();
    ui->label_video_lng->clear();
    ui->label_video_size->clear();
    ui->label_video_title->clear();

    ui->label_video_dar->clear();
    ui->label_video_var->clear();
    ui->label_video_par->clear();
    ui->label_video_bitrate_gross->clear();
    ui->label_video_bitrate_lowest->clear();
    ui->label_video_bitrate_highest->clear();
    ui->label_video_bitratemode->clear();
    ui->label_video_codec->clear();
    ui->label_video_compression_ratio->clear();
    ui->label_video_definition->clear();
    ui->label_video_definition_visible->clear();

    ui->label_video_encoder->clear();
    ui->label_video_fcc->clear();
    ui->label_video_framerate->clear();
    ui->label_video_framerate_mode->clear();
    ui->label_video_frameduration->clear();

    ui->label_video_color_depth->clear();
    ui->label_video_color_matrix->clear();
    ui->label_video_color_space->clear();
    ui->label_video_color_subsampling->clear();

    ui->label_video_framecount->clear();
    ui->label_video_samplecount->clear();
    ui->label_video_samplerepart->clear();

    // Subtitles tab
    ui->comboBox_sub_selector->clear();
    ui->label_sub_codec->clear();
    ui->label_sub_encoding->clear();
    ui->label_sub_id->clear();
    ui->label_sub_lng->clear();
    ui->label_sub_size->clear();
    ui->label_sub_title->clear();
    ui->textBrowser_sub->clear();

    // Others tab

    // Export tab
    exportDatas.clear();
    exportFormat = EXPORT_TEXT;
    ui->comboBox_export_details->setCurrentIndex(0);
    ui->comboBox_export_formats->setCurrentIndex(0);
    ui->lineEdit_export_filename->clear();
    ui->textBrowser_export->clear();
}

int MainWindow::printDatas()
{
    int retcode = 0;

    handleTabWidget();

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        // Combobox icon
        if (media->tracks_video_count > 0)
            ui->file_comboBox->setItemIcon(ui->file_comboBox->currentIndex(), QIcon(":/icons/icons/video-x-generic.svg"));
         else if (media->tracks_audio_count > 0)
            ui->file_comboBox->setItemIcon(ui->file_comboBox->currentIndex(), QIcon(":/icons/icons/audio-x-wav.svg"));
         else
            ui->file_comboBox->setItemIcon(ui->file_comboBox->currentIndex(), QIcon(":/icons/icons/dialog-error.svg"));

        // General infos
        ui->label_info_filename->setText(QString::fromLocal8Bit(media->file_name) + "." + QString::fromLocal8Bit(media->file_extension));
        ui->label_info_filename->setToolTip(QString::fromLocal8Bit(media->file_name));
        ui->label_info_fullpath->setText(QString::fromLocal8Bit(media->file_path));
        ui->label_info_fullpath->setToolTip(QString::fromLocal8Bit(media->file_path));
        ui->label_info_container->setText(getContainerString(media->container, 1));
        ui->label_info_filesize->setText(getSizeString(media->file_size));
        ui->label_info_duration->setText(getDurationString(media->duration));

        if (media->creation_app)
        {
            ui->label_6->setVisible(false);
            ui->label_info_creationapp->setVisible(true);
            ui->label_info_creationapp->setText(QString::fromLocal8Bit(media->creation_app));
        }
        else
        {
            ui->label_6->setVisible(false);
            ui->label_info_creationapp->setVisible(false);
        }

        if (media->creation_time)
        {
            ui->label_5->setVisible(true);
            ui->label_info_creationdate->setVisible(true);

            if (media->container == CONTAINER_MP4)
            {
                QDate date(1904, 1, 1);
                QTime time(0, 0, 0, 0);
                QDateTime datetime(date, time);
                datetime = datetime.addSecs(media->creation_time);
                ui->label_info_creationdate->setText(datetime.toString("dddd d MMMM yyyy, hh:mm:ss"));
            }
        }
        else
        {
            ui->label_5->setVisible(false);
            ui->label_info_creationdate->setVisible(false);
        }

        // Container efficiency
        {
            uint64_t trackssize = 0;
            for (int i = 0; i < 16; i++)
            {
                if (media->tracks_audio[i])
                    trackssize += media->tracks_audio[i]->stream_size;
                if (media->tracks_video[i])
                    trackssize += media->tracks_video[i]->stream_size;
                if (media->tracks_subt[i])
                    trackssize += media->tracks_subt[i]->stream_size;
                //if (media->tracks_others[i])
                //    trackssize += media->tracks_others[i]->stream_size;
            }

            uint64_t overhead = media->file_size - trackssize;
            double overheadpercent = (static_cast<double>(overhead) / static_cast<double>(media->file_size)) * 100.0;

            if (overheadpercent < 0.01)
                ui->label_info_container_overhead->setText("<b>~0.01%</b>   >   " + getSizeString(overhead));
            else if (overheadpercent <= 100)
                ui->label_info_container_overhead->setText("<b>" + QString::number(overheadpercent, 'f', 2) + "%</b>   >   " + getSizeString(overhead));
        }

        // EXPORT
        ////////////////////////////////////////////////////////////////////////

        generateExportDatas();

        // AUDIO
        ////////////////////////////////////////////////////////////////////////

        if (media->tracks_audio_count == 0 || media->tracks_audio[0] == NULL)
        {
            ui->groupBox_audio->hide();
        }
        else
        {
            ui->groupBox_audio->show();

            const BitstreamMap_t *t = media->tracks_audio[0];

            ui->label_info_audio_size->setText(getTrackSizeString(t, media->file_size));
            ui->label_info_audio_codec->setText(getCodecString(stream_AUDIO, t->stream_codec, true));
            ui->label_info_audio_duration->setText(getDurationString(t->duration_ms));
            ui->label_info_audio_bitrate->setText(getBitrateString(t->bitrate));
            ui->label_info_audio_samplingrate->setText(QString::number(t->sampling_rate) + " Hz");
            ui->label_info_audio_channels->setText(QString::number(t->channel_count));

            if (t->bit_per_sample)
            {
                ui->label_info_audio_bitpersample->setText(QString::number(t->bit_per_sample) + " bits");
            }
            ui->label_info_audio_bitratemode->setText(getBitrateModeString(t->bitrate_mode));

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
                        ui->comboBox_audio_selector->addItem(text);
                    }
                }
            }
            else
            {
                ui->comboBox_audio_selector->hide();
            }

            printAudioDetails();
        }

        // VIDEO
        ////////////////////////////////////////////////////////////////////////

        if (media->tracks_video_count == 0 || media->tracks_video[0] == NULL)
        {
            ui->groupBox_video->hide();
        }
        else
        {
            ui->groupBox_video->show();

            const BitstreamMap_t *t = media->tracks_video[0];

            ui->label_info_video_codec->setText(getCodecString(stream_VIDEO, t->stream_codec, true));
            ui->label_info_video_duration->setText(getDurationString(t->duration_ms));
            ui->label_info_video_bitrate->setText(getBitrateString(t->bitrate));
            ui->label_info_video_definition->setText(QString::number(t->width) + " x " + QString::number(t->height));
            ui->label_info_video_dar->setText(getAspectRatioString(t->display_aspect_ratio));
            ui->label_info_video_framerate->setText(QString::number(t->framerate) + " fps");
            ui->label_info_video_framerate_mode->setText(getFramerateModeString(t->framerate_mode));
            ui->label_info_video_size->setText(getTrackSizeString(t, media->file_size));
            ui->label_info_video_bitratemode->setText(getBitrateModeString(t->bitrate_mode));

            if (media->tracks_video_count > 1)
            {
                ui->comboBox_video_selector->show();
                for (unsigned i = 0; i < media->tracks_video_count; i++)
                {
                    if (media->tracks_video[i])
                    {
                        QString text = "Video track #" + QString::number(i);
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

        int stid = 0;
        if (media->tracks_subt[stid] != NULL)
        {
            // TODO
        }

        if (media->tracks_subtitles_count > 1)
        {
            ui->comboBox_sub_selector->show();
            for (unsigned i = 0; i < media->tracks_subtitles_count; i++)
            {
                if (media->tracks_subt[i])
                {
                    QString text = "Subtitles track #" + QString::number(i);
                    if (i != media->tracks_subt[i]->track_id)
                        text += " (internal id #" + QString::number(media->tracks_subt[i]->track_id) + ")";
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
    else
    {
        retcode = 0;
    }

    return retcode;
}

int MainWindow::printAudioDetails()
{
    int retcode = 0;

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        int atid = ui->comboBox_audio_selector->currentIndex();
        if (atid < 0) atid = 0;

        BitstreamMap_t *t = media->tracks_audio[atid];

        if (t != NULL)
        {
            ui->groupBox_tab_audio->setTitle(tr("Audio track") + " #" + QString::number(atid + 1));

            ui->label_audio_id->setText(QString::number(t->track_id));

            if (t->track_title)
            {
                ui->label_audio_title->setText(QString::fromLocal8Bit(t->track_title));
            }
            if (t->stream_encoder)
            {
                ui->label_audio_encoder->setText(QString::fromLocal8Bit(t->stream_encoder));
            }

            ui->label_audio_size->setText(getTrackSizeString(t, media->file_size, true));
            ui->label_audio_codec->setText(getCodecString(stream_AUDIO, t->stream_codec, true));

            char fcc_str[4];
            {
                fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
            }
            ui->label_audio_fcc->setText(QString::fromLatin1(fcc_str, 4));

            ui->label_audio_duration->setText(getDurationString(t->duration_ms));

            ui->label_audio_bitrate_gross->setText(getBitrateString(t->bitrate));
            ui->label_audio_bitratemode->setText(getBitrateModeString(t->bitrate_mode));

            ui->label_audio_samplingrate->setText(QString::number(t->sampling_rate) + " Hz");
            ui->label_audio_channels->setText(QString::number(t->channel_count));

            if (t->bit_per_sample)
            {
                ui->label_audio_bitpersample->setText(QString::number(t->bit_per_sample) + " bits");
            }

            uint64_t rawsize = t->sampling_rate * t->channel_count * (t->bit_per_sample / 8);
            rawsize *= t->duration_ms;
            rawsize /= 1024.0;

            uint64_t ratio = round(static_cast<double>(rawsize) / static_cast<double>(t->stream_size));
            ui->label_audio_compression_ratio->setText(QString::number(ratio) + ":1");

            ui->label_audio_samplecount->setText(QString::number(t->sample_count));
            //ui->label_audio_framecount->setText(QString::number(t->frame_count));
            //ui->label_audio_frameduration->setText(QString::number(t->frame_duration));
        }
    }

    return retcode;
}

int MainWindow::printVideoDetails()
{
    int retcode = 0;

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
        int vtid = ui->comboBox_video_selector->currentIndex();
        if (vtid < 0) vtid = 0;

        const BitstreamMap_t *t = media->tracks_video[vtid];

        if (t != NULL)
        {
            ui->groupBox_tab_video->setTitle(tr("Video track") + " #" + QString::number(vtid + 1));

            ui->label_video_id->setText(QString::number(t->track_id));

            if (t->track_title)
            {
                ui->label_video_title->setText(QString::fromLocal8Bit(t->track_title));
            }
            if (t->stream_encoder)
            {
                ui->label_video_encoder->setText(QString::fromLocal8Bit(t->stream_encoder));
            }

            ui->label_video_size->setText(getTrackSizeString(t, media->file_size, true));
            ui->label_video_codec->setText(getCodecString(stream_VIDEO, t->stream_codec, true));

            char fcc_str[4];
            {
                fcc_str[3] = (t->stream_fcc >>  0) & 0xFF;
                fcc_str[2] = (t->stream_fcc >>  8) & 0xFF;
                fcc_str[1] = (t->stream_fcc >> 16) & 0xFF;
                fcc_str[0] = (t->stream_fcc >> 24) & 0xFF;
            }
            ui->label_video_fcc->setText(QString::fromLatin1(fcc_str, 4));

            ui->label_video_duration->setText(getDurationString(t->duration_ms));

            ui->label_video_bitrate_gross->setText(getBitrateString(t->bitrate));
            ui->label_video_bitratemode->setText(getBitrateModeString(t->bitrate_mode));

            ui->label_video_definition->setText(QString::number(t->width) + " x " + QString::number(t->height));
            ui->label_video_dar->setText(getAspectRatioString(t->display_aspect_ratio, true));
            if (t->video_aspect_ratio != t->display_aspect_ratio)
            {
                ui->label_38->setVisible(true);
                ui->label_81->setVisible(true);
                ui->label_video_var->setVisible(true);
                ui->label_video_par->setVisible(true);
                ui->label_video_var->setText(getAspectRatioString(t->video_aspect_ratio, false));
                ui->label_video_par->setText(getAspectRatioString(t->pixel_aspect_ratio, false));
            }
            else
            {
                ui->label_38->setVisible(false);
                ui->label_81->setVisible(false);
                ui->label_video_var->setVisible(false);
                ui->label_video_par->setVisible(false);
            }

            ui->label_video_color_depth->setText(QString::number(t->color_depth) + " bits");
            ui->label_video_color_subsampling->setText(QString::number(t->color_subsampling));

            if (t->color_encoding == CLR_RGB)
                ui->label_video_color_space->setText("RGB");
            else if (t->color_encoding == CLR_YCgCo)
                ui->label_video_color_space->setText("YCgCo");
            else if (t->color_encoding == CLR_YCbCr)
                ui->label_video_color_space->setText("YCbCr");
            else
                ui->label_video_color_space->setText("YCbCr (best guess)");

            if (t->color_matrix == CM_bt470)
                ui->label_video_color_matrix->setText("Rec. 470");
            else if (t->color_matrix == CM_bt601)
                ui->label_video_color_matrix->setText("Rec. 601");
            else if (t->color_matrix == CM_bt709)
                ui->label_video_color_matrix->setText("Rec. 709");
            else if (t->color_matrix == CM_bt2020)
                ui->label_video_color_matrix->setText("Rec. 2020");

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

            double framerate = t->framerate;
            if (framerate < 1.0)
            {
                if (t->duration_ms && t->sample_count)
                {
                    framerate = static_cast<double>(t->sample_count / (static_cast<double>(t->duration_ms) / 1000.0));
                }
            }
            double frameduration = 1000.0 / framerate; // in ms

            QString samplecount = "<b>" + QString::number(t->sample_count) + "</b>";
            QString framecount = "<b>" + QString::number(t->frame_count) + "</b>";
            QString samplerepartition;

            if (t->frame_count_idr)
            {
                framecount += "      (" + QString::number(t->frame_count_idr) + " IDR  /  ";
                framecount += QString::number(t->frame_count - t->frame_count_idr) + " others)";

                double idr_ratio = static_cast<double>(t->frame_count_idr) / static_cast<double>(t->sample_count) * 100.0;
                samplerepartition = tr("IDR frames makes <b>") + QString::number(idr_ratio, 'g', 2) + "%</b> " + tr("of the samples");
                samplerepartition += "<br>Statistically, one every <b>X</b> ms";

                ui->label_video_samplerepart->setText(samplerepartition);
            }

            ui->label_video_samplecount->setText(samplecount);
            ui->label_video_framecount->setText(framecount);

            ui->label_video_framerate->setText(QString::number(framerate) + " fps");
            ui->label_video_framerate_mode->setText(getFramerateModeString(t->framerate_mode));
            ui->label_video_frameduration->setText(QString::number(frameduration, 'g', 4) + " ms");

            uint64_t rawsize = t->width * t->height * (t->color_depth / 8);
            rawsize *= t->sample_count;
            uint64_t ratio = round(static_cast<double>(rawsize) / static_cast<double>(t->stream_size));
            ui->label_video_compression_ratio->setText(QString::number(ratio) + ":1");
        }
    }

    return retcode;
}

int MainWindow::printSubtitlesDetails()
{
    int retcode = 0;

    MediaFile_t *media = currentMediaFile();

    if (media)
    {
/*
        ui->label_sub_id->setText();
        ui->label_sub_codec->setText();
        ui->label_sub_encoding->setText();
        ui->label_sub_lng->setText();
        ui->label_sub_size->setText();
        ui->label_sub_title->setText();
*/
    }

    return retcode;
}
