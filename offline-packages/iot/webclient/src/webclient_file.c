/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-26     chenyong     modify log information
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rtthread.h>
#include <webclient.h>

#ifdef RT_USING_DFS
#include <unistd.h>
#include <fcntl.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME               "web.file"
#ifdef WEBCLIENT_DEBUG
#define DBG_LEVEL                      DBG_LOG
#else
#define DBG_LEVEL                      DBG_INFO
#endif /* WEBCLIENT_DEBUG */
#define DBG_COLOR
#include <rtdbg.h>

static const char *progress_bar (int progress)
{
    static char bar[51];
    int i;

    for (i = 0; i < 50; i++)
    {
        if (i < progress / 2)
        {
            bar[i] = '=';
        }
        else if (i == progress / 2)
        {
            bar[i] = '>';
        }
        else
        {
            bar[i] = ' ';
        }
    }

    bar[50] = '\0';

    return bar;
}

/**
 * send GET request and store response data into the file.
 *
 * @param URI input server address
 * @param filename store response date to filename
 *
 * @return <0: GET request failed
 *         =0: success
 */
int webclient_get_file(const char* URI, const char* filename)
{
    int fd = -1, rc = WEBCLIENT_OK;
    size_t offset;
    int length, total_length = 0;
    unsigned char *ptr = RT_NULL;
    struct webclient_session* session = RT_NULL;
    int resp_status = 0;

    int filtered_speed = 0;
    int last_downloaded = 0;
    rt_tick_t last_tick = rt_tick_get();

    session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
    if(session == RT_NULL)
    {
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    if ((resp_status = webclient_get(session, URI)) != 200)
    {
        LOG_E("get file failed, wrong response: %d (-0x%X).", resp_status, resp_status);
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        LOG_E("get file failed, open file(%s) error.", filename);
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    ptr = (unsigned char *) web_malloc(WEBCLIENT_RESPONSE_BUFSZ);
    if (ptr == RT_NULL)
    {
        LOG_E("get file failed, no memory for response buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    if (session->content_length < 0)
    {
        while (1)
        {
            length = webclient_read(session, ptr, WEBCLIENT_RESPONSE_BUFSZ);
            if (length > 0)
            {
                write(fd, ptr, length);
                total_length += length;
                LOG_RAW(">");
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        for (offset = 0; offset < (size_t) session->content_length;)
        {
            length = webclient_read(session, ptr,
                    session->content_length - offset > WEBCLIENT_RESPONSE_BUFSZ ?
                            WEBCLIENT_RESPONSE_BUFSZ : session->content_length - offset);

            if (length > 0)
            {
                /* Write data */
                write(fd, ptr, length);
                total_length += length;
                
                /* Show download progress */
                rt_tick_t current_tick;
                current_tick = rt_tick_get_millisecond();
                rt_tick_t elapsed_time = current_tick - last_tick;
                if (elapsed_time >= (RT_TICK_PER_SECOND / 10))
                {
                    int progress = (rt_uint64_t)total_length * 100UL / (rt_uint64_t)session->content_length;
                    int download_speed = (total_length - last_downloaded) * 1000 / (elapsed_time * 1024);
                    last_downloaded = total_length;
                    last_tick = current_tick;
                    filtered_speed = (filtered_speed * 3 + download_speed) / 4;
                    rt_kprintf("\033[2K\rDownloading: [%-50s] %d%%", progress_bar(progress), progress);                    
                    rt_kprintf(" %u | %d KB/s", total_length, filtered_speed);
                }
            }
            else
            {
                break;
            }

            offset += length;
        }
    }

    if (total_length == session->content_length)
    {
        rt_kprintf("\033[2K\rDownloading: [%-50s] %d%%", progress_bar(100), 100);                    
        rt_kprintf(" %u | %d KB/s", total_length, filtered_speed);
        rt_kprintf("\n");
    }
    else
    {
        rt_kprintf("Data receiving incomplete, need to receive length: %u, actual receiving: %u", session->content_length, total_length);
    }

    if (total_length)
    {
        LOG_D("save %d bytes.", total_length);
    }

__exit:
    if (fd >= 0)
    {
        close(fd);
    }

    if (session != RT_NULL)
    {
        webclient_close(session);
        session = RT_NULL;
    }

    if (ptr != RT_NULL)
    {
        web_free(ptr);
    }

    return rc;
}

/**
 * post file to http server.
 *
 * @param URI input server address
 * @param filename post data filename
 * @param form_data  form data
 *
 * @return <0: POST request failed
 *         =0: success
 */
int webclient_post_file(const char* URI, const char* filename,
        const char* form_data)
{
    size_t length;
    char boundary[60];
    int fd = -1, rc = WEBCLIENT_OK;
    char *header = RT_NULL, *header_ptr;
    unsigned char *buffer = RT_NULL, *buffer_ptr;
    struct webclient_session* session = RT_NULL;
    int resp_data_len = 0;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0)
    {
        LOG_D("post file failed, open file(%s) error.", filename);
        rc = -WEBCLIENT_FILE_ERROR;
        goto __exit;
    }

    /* get the size of file */
    length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    buffer = (unsigned char *) web_calloc(1, WEBCLIENT_RESPONSE_BUFSZ);
    if (buffer == RT_NULL)
    {
        LOG_D("post file failed, no memory for response buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    header = (char *) web_calloc(1, WEBCLIENT_HEADER_BUFSZ);
    if (header == RT_NULL)
    {
        LOG_D("post file failed, no memory for header buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }
    header_ptr = header;

    /* build boundary */
    web_snprintf(boundary, sizeof(boundary), "----------------------------%012d", rt_tick_get());

    /* build encapsulated mime_multipart information*/
    buffer_ptr = buffer;
    /* first boundary */
    buffer_ptr += web_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer), "--%s\r\n", boundary);
    buffer_ptr += web_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer),
            "Content-Disposition: form-data; %s\r\n", form_data);
    buffer_ptr += web_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer),
            "Content-Type: application/octet-stream\r\n\r\n");
    /* calculate content-length */
    length += buffer_ptr - buffer;
    length += strlen(boundary) + 8; /* add the last boundary */

    /* build header for upload */
    header_ptr += web_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Length: %d\r\n", length);
    header_ptr += web_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Type: multipart/form-data; boundary=%s\r\n", boundary);

    session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
    if(session == RT_NULL)
    {
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    strncpy(session->header->buffer, header, strlen(header));
    session->header->length = strlen(session->header->buffer);

    rc = webclient_post(session, URI, NULL, 0);
    if(rc < 0)
    {
        goto __exit;
    }

    /* send mime_multipart */
    webclient_write(session, buffer, buffer_ptr - buffer);

    /* send file data */
    while (1)
    {
        length = read(fd, buffer, WEBCLIENT_RESPONSE_BUFSZ);
        if (length <= 0)
        {
            break;
        }

        webclient_write(session, buffer, length);
    }

    /* send last boundary */
    web_snprintf((char*) buffer, WEBCLIENT_RESPONSE_BUFSZ, "\r\n--%s--\r\n", boundary);
    webclient_write(session, buffer, strlen(boundary) + 8);

    extern int webclient_handle_response(struct webclient_session *session);
    if( webclient_handle_response(session) != 200)
    {
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    resp_data_len = webclient_content_length_get(session);
    if (resp_data_len > 0)
    {
        int bytes_read = 0;

        web_memset(buffer, 0x00, WEBCLIENT_RESPONSE_BUFSZ);
        do
        {
            bytes_read = webclient_read(session, buffer,
                resp_data_len < WEBCLIENT_RESPONSE_BUFSZ ? resp_data_len : WEBCLIENT_RESPONSE_BUFSZ);
            if (bytes_read <= 0)
            {
                break;
            }
            resp_data_len -= bytes_read;
        } while(resp_data_len > 0);
    }

__exit:
    if (fd >= 0)
    {
        close(fd);
    }

    if (session != RT_NULL)
    {
        webclient_close(session);
        session = RT_NULL;
    }

    if (buffer != RT_NULL)
    {
        web_free(buffer);
    }

    if (header != RT_NULL)
    {
        web_free(header);
    }

    return rc;
}

int wget(int argc, char** argv)
{
    if (argc != 3)
    {
        rt_kprintf("Please using: wget <URI> <filename>\n");
        return -1;
    }

    webclient_get_file(argv[1], argv[2]);
    return 0;
}

#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT(wget, Get file by URI: wget <URI> <filename>.);
#endif /* FINSH_USING_MSH */

#endif /* RT_USING_DFS */
