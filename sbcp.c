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
    int             fd1, fd2;
    int             num_blocks;
    int             block_size;
    int             i;
    int             sparse_blocks;
    int             zero_blocks;
    int             copied_blocks;
    int             percent = -1;
    unsigned char   *buffer1;

    if (argc != 3) {
        fprintf(stderr, "%s a fast sparse-aware binary file copy.\n", argv[0]);
        fprintf(stderr, "Usage: %s FILE1 FILE2\n", argv[0]);
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


    if ((fd2 = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0777)) < 0) {
        fprintf(stderr, "Cannot open %s\n", argv[3]);
        close(fd1);
        exit(EXIT_FAILURE);
    }

    buffer1=malloc(block_size);
    if (!buffer1) {
        fprintf(stderr, "Cannot allocate buffer\n");
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }

    sparse_blocks = 0;
    zero_blocks = 0;
    //        different_blocks = 0;
    copied_blocks = 0;

    for (i=0; i<num_blocks; i++) {
        int block = i;
        int nread = 0;

        if (ioctl(fd1, FIBMAP, &block)) {
            printf("ioctl failed: %s in block %d, are you root ?\n", strerror(errno), i);

            close(fd1);
            close(fd2);
            exit(EXIT_FAILURE);
        }

        if (percent != ((i+1)*100 / num_blocks)){
            percent = (i+1)*100 / num_blocks;

            printf("%08d/%08d (%d%%), S:%08d, Z:%08d, C:%08d  \r",
                    i+1, num_blocks, percent, sparse_blocks, zero_blocks, copied_blocks);
            fflush(0);
        }

        /* No source block: skip*/
        if (!block) {
            if (lseek(fd1, block_size, SEEK_CUR) < 0 ||
                    lseek(fd2, block_size, SEEK_CUR) < 0 ) {
                fprintf(stderr, "Seek failed.\n");
                close(fd1);
                close(fd2);
                exit(EXIT_FAILURE);
            }
            sparse_blocks++;
            continue;
        }

        /* Check for zero blocks: skip if empty*/
        else {
            nread = read(fd1, buffer1, block_size);

            if (nread < block_size) {
                if (i != (num_blocks -1)){
                    fprintf(stderr, "Unexpected short read.\n");
                    close(fd1);
                    close(fd2);
                    exit(EXIT_FAILURE);
                }
            }

            if (nread == -1) {
                fprintf(stderr, "Cannot read block %d on %s\n", i, argv[2]);
            }
            else {
                int n;

                for (n=0; n<nread; n++) {
                    if (buffer1[n])
                        break;
                }

                if (n==nread){
                    if (lseek(fd2, nread, SEEK_CUR) < 0 ) {
                        fprintf(stderr, "Seek failed.\n");
                        close(fd1);
                        close(fd2);
                        exit(EXIT_FAILURE);
                    }
                    zero_blocks++;
                    //                                    printf("S");
                    continue;
                }
                else {
                    if (write(fd2, buffer1, nread) != nread) {
                        fprintf(stderr, "Cannot write block %d on %s\n", i, argv[3]);
                    }
                    else {
                        copied_blocks++;
                        continue;
                    }
                }
            }
        }
    }

    close(fd1);
    free(buffer1);

    /* Finalize the file */
    ftruncate(fd2,statinfo1.st_size);
    fchown(fd2, statinfo1.st_uid, statinfo1.st_gid);
    fchmod(fd2, statinfo1.st_mode);

    close(fd2);

    printf("\n");

    printf("Report:\n");
    printf("%d byte block size\n", block_size);
    printf("  total %08d blocks\n",num_blocks);
    printf("Skipped %08d sparse blocks\n",sparse_blocks);
    printf("  found %08d zero blocks (now sparse)\n",zero_blocks);
    printf(" copied %08d blocks\n",copied_blocks);
    printf("  ------------------------------------\n");
    printf("  Sum:  %08d blocks accounted for.\n",(sparse_blocks+zero_blocks+copied_blocks));

    exit(EXIT_SUCCESS);
}
