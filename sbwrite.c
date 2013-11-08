/*
 * This file is part of sparsefileutils.
 *
 * Copyright (C) 2012  D.Herrendoerfer
 *
 *   sparsefileutils is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   sparsefileutils is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>

int main(int argc, char **argv) {
    int             fd2;
    int             block_size;
    int             i;
    int             zero_blocks;
    int             copied_blocks;
    long            filesize;
    unsigned char   *buffer1;

    if (argc != 2) {
        fprintf(stderr, "%s a fast sparse-aware binary file writer.\n", argv[0]);
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((fd2 = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0777)) < 0) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd2, FIGETBSZ, &block_size) < 0) {
        fprintf(stderr, "Cannot get block size\n");
        exit(EXIT_FAILURE);
    }

    buffer1=malloc(block_size);
    if (!buffer1) {
        fprintf(stderr, "Cannot allocate buffer\n");
        close(fd2);
        exit(EXIT_FAILURE);
    }

    zero_blocks = 0;
    copied_blocks = 0;

    filesize = 0;

    i=0;
    while ( 1 == 1 ) {
        int nread = 0;
        int tread = 0;

        /* Check for zero blocks: skip if empty*/

        while (tread < block_size) {
            nread = read(STDIN_FILENO, buffer1+tread, block_size-tread);
            tread += nread;

            if (nread == 0)
                break;
        }
        filesize += tread;

        if (tread == 0) {
            // End of file
            break;
        }
        else {
            i++; // counting the numbers of blocks
            int n;

            for (n=0; n<tread; n++) {
                if (buffer1[n])
                    break;
            }

            if (n==tread){
                if (lseek(fd2, tread, SEEK_CUR) < 0 ) {
                    fprintf(stderr, "Seek failed.\n");
                    close(fd2);
                    exit(EXIT_FAILURE);
                }
                zero_blocks++;
                //                                    printf("S");
                continue;
            }
            else {
                if (write(fd2, buffer1, tread) != tread) {
                    fprintf(stderr, "Cannot write block %d on %s\n", i, argv[3]);
                }
                else {
                    copied_blocks++;
                    continue;
                }
            }
        }
    }

    free(buffer1);

    ftruncate(fd2,filesize);
    close(fd2);

    printf("\n");

    printf("Report:\n");
    printf("%d byte block size\n", block_size);
    printf("  total %08d blocks\n",i);
    printf("  found %08d zero blocks (now sparse)\n",zero_blocks);
    printf(" copied %08d blocks\n",copied_blocks);
    printf("  ------------------------------------\n");
    printf("  Sum:  %08d blocks accounted for.\n",(zero_blocks+copied_blocks));

    exit(EXIT_SUCCESS);
}
