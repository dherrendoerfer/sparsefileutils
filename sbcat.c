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
    struct stat     statinfo1;
    int             fd1;
    int             num_blocks;
    int             block_size;
    int             i;
    unsigned char   *buffer1, *buffer2;

    if (argc != 2) {
        fprintf(stderr, "%s a fast sparse-aware cat.\n", argv[0]);
        fprintf(stderr, "Usage: %s FILE1\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((fd1 = open(argv[1], O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd1, FIGETBSZ, &block_size) < 0) {
        fprintf(stderr, "Cannot get block size\n");
        close(fd1);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd1, &statinfo1) < 0) {
        fprintf(stderr, "Cannot stat %s\n", argv[1]);
        close(fd1);
        exit(EXIT_FAILURE);
    }

    num_blocks = (statinfo1.st_size + block_size - 1) / block_size;

    buffer1=malloc(block_size);
    if (!buffer1) {
        fprintf(stderr, "Cannot allocate buffer\n");
        close(fd1);
        exit(EXIT_FAILURE);
    }

    buffer2=malloc(block_size);
    if (!buffer1) {
        fprintf(stderr, "Cannot allocate buffer\n");
        close(fd1);
        exit(EXIT_FAILURE);
    }

    memset(buffer2,0,4096);

    for (i=0; i<num_blocks; i++) {
        int block = i;
        int nread = 0;

        if (ioctl(fd1, FIBMAP, &block)) {
            printf("ioctl failed: %s in block %d, are you root ?\n", strerror(errno), i);

            close(fd1);
            exit(EXIT_FAILURE);
        }

        /* No source block: skip, just pump out block_size of zeroes*/
        if (!block) {
            if (lseek(fd1, block_size, SEEK_CUR) < 0 ) {
                fprintf(stderr, "Seek failed.\n");
                close(fd1);
                exit(EXIT_FAILURE);
            }

            if (write(1, buffer2, block_size) != block_size) {
                fprintf(stderr, "Cannot write block %d on STDOUT\n", i);
                close(fd1);
                exit(EXIT_FAILURE);
            }
            fflush(stdout);
            continue;
        }

        nread = read(fd1, buffer1, block_size);

        if (nread < block_size) {
            if (i != (num_blocks -1)){
                fprintf(stderr, "Unexpected short read.\n");
                close(fd1);
                exit(EXIT_FAILURE);
            }
        }

        if (write(1, buffer1, nread) != nread) {
            fprintf(stderr, "Cannot write block %d on STDOUT\n", i);
            close(fd1);
            exit(EXIT_FAILURE);
        }
    }

    close(fd1);
    fflush(stdout);
    exit(EXIT_FAILURE);
}

