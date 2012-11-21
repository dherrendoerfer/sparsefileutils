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
    struct stat     statinfo1,statinfo2;
    int             fd1, fd2, fd3;
    int             num_blocks;
    int             block_size;
    int             i;
    int             sparse_blocks;
    int             zero_blocks;
    int             identical_blocks;
    int             different_blocks;
    int             percent = -1;
    unsigned char   *buffer1, *buffer2;

    if (argc != 4) {
        fprintf(stderr, "%s a fast sparse-aware binary file differ.\n", argv[0]);
        fprintf(stderr, "Usage: %s FILE2 FILE1 DIFF\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((fd1 = open(argv[1], O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if ((fd2 = open(argv[2], O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", argv[2]);
        close(fd1);
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd1, FIGETBSZ, &block_size) < 0) {
        fprintf(stderr, "Cannot get block size\n");
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd1, &statinfo1) < 0) {
        fprintf(stderr, "Cannot stat %s\n", argv[1]);
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd1, &statinfo2) < 0) {
        fprintf(stderr, "Cannot stat %s\n", argv[2]);
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }
    num_blocks = (statinfo1.st_size + block_size - 1) / block_size;


    if ((fd3 = open(argv[3], O_WRONLY|O_CREAT|O_TRUNC, 0777)) < 0) {
        fprintf(stderr, "Cannot open %s\n", argv[3]);
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }

    buffer1=malloc(block_size);
    buffer2=malloc(block_size);
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Cannot allocate buffer\n");
        close(fd1);
        close(fd2);
        close(fd3);
        exit(EXIT_FAILURE);
    }

    sparse_blocks = 0;
    zero_blocks = 0;
    identical_blocks = 0;
    different_blocks = 0;

    for (i=0; i<num_blocks; i++) {
        int block = i;
        int sblock = i;
        int nread = 0;

        if (ioctl(fd1, FIBMAP, &block) ||
                ioctl(fd2, FIBMAP, &sblock)) {
            printf("ioctl failed: %s in block %d, are you root ?\n", strerror(errno), i);

            close(fd1);
            close(fd2);
            close(fd3);
            exit(EXIT_FAILURE);
        }

        if (percent != ((i+1)*100 / num_blocks)){
            percent = (i+1)*100 / num_blocks;

            printf("%08d/%08d (%d%%), S:%08d, Z:%08d, I:%08d ,M:%08d \r",
                    i+1, num_blocks, percent, sparse_blocks, zero_blocks, identical_blocks, different_blocks);
            fflush(0);

        }

        if (!block && !sblock) {
            if (lseek(fd1, block_size, SEEK_CUR) < 0 ||
                    lseek(fd2, block_size, SEEK_CUR) < 0 ||
                    lseek(fd3, block_size, SEEK_CUR) < 0 ) {
                fprintf(stderr, "Seek failed.\n");
                close(fd1);
                close(fd2);
                close(fd3);
                exit(EXIT_FAILURE);
            }
            //                        printf("S");
            sparse_blocks++;
        }
        else {
            //                        printf("%10d\t", block);
            //                        printf(".");

            nread = read(fd1, buffer1, block_size);

            if (nread < block_size) {
                if (i != (num_blocks -1)){
                    fprintf(stderr, "Unexpected short read.\n");
                    close(fd1);
                    close(fd2);
                    close(fd3);
                    exit(EXIT_FAILURE);
                }
            }

            if (nread == -1) {
                fprintf(stderr, "Cannot read block %d on %s\n", i, argv[1]);
            }

            //check for null block ( new block null, old block sparse)
            if (!sblock) {
                int n;

                for (n=0; n<nread; n++) {
                    if (buffer1[n])
                        break;
                }

                if (n==nread){
                    if (lseek(fd2, block_size, SEEK_CUR) < 0 ||
                            lseek(fd3, block_size, SEEK_CUR) < 0 ) {
                        fprintf(stderr, "Seek failed.\n");
                        close(fd1);
                        close(fd2);
                        close(fd3);
                        exit(EXIT_FAILURE);
                    }
                    zero_blocks++;
                    continue;
                }
            }

            if (read(fd2, buffer2, nread) != nread) {
                fprintf(stderr, "Cannot read block %d on %s\n", i, argv[2]);
            }
            else {
                int n;

                // check for identical blocks
                for (n=0; n<nread; n++) {
                    if (buffer1[n] != buffer2[n])
                        break;
                }

                if (n==nread){
                    if (lseek(fd3, nread, SEEK_CUR) < 0 ) {
                        fprintf(stderr, "Seek failed.\n");
                        close(fd1);
                        close(fd2);
                        close(fd3);
                        exit(EXIT_FAILURE);
                    }
                    identical_blocks++;
                    continue;
                }
            }

            /* Now there are definitely 2 different buffers */

            if (write(fd3, buffer1, nread) != block_size) {
                fprintf(stderr, "Cannot write block %d on %s\n", i, argv[3]);
            }
            else {
                different_blocks++;
                //                                printf("M");
            }
        }
    }

    close(fd1);
    close(fd2);
    ftruncate(fd3,statinfo1.st_size);
    close(fd3);

    free(buffer1);
    free(buffer2);

    printf("\n");

    printf("Report:\n");
    printf("%d byte block size\n", block_size);
    printf("  total %08d blocks\n",num_blocks);
    printf("Skipped %08d sparse blocks\n",sparse_blocks);
    printf("  found %08d zero blocks\n",zero_blocks);
    printf("  found %08d identical blocks\n",identical_blocks);
    printf("  wrote %08d different blocks\n",different_blocks);
    printf("  ------------------------------------\n");
    printf("  Sum:  %08d blocks accounted for.\n",(sparse_blocks+zero_blocks+identical_blocks+different_blocks));

    exit(EXIT_SUCCESS);
}
