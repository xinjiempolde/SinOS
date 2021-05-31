#include <gui/window.h>
#include <gui/vga.h>
#include <libc/string.h>
#include <gui/fileexplore.h>
void creat_file_icon(uint8_t* buf, int weight, int x, int y)
{
    	fill_rect(buf, weight, 12+x, 10+y, 17, 5, DARK_BLUE);
		fill_rect(buf, weight, 8+x, 15+y, 45, 26, BRIGHT_BLUE);
}
void init_file_list(uint8_t* buf, int weight, int height, char* title,char **file_name_list,int length)
{
    init_window_buf(buf,weight,height,title);
    	int num=0;
        int text_length;
        // 行数 height/70
	for(int i=0;i<height/70;i++)
	{
		for(int j=0;j<weight/INTE_FILE_ICON_WIDTH;j++)
		{
				int x= LEFT_MARGIN+INTE_FILE_ICON_WIDTH*j;
				int y= TOP_MARGIN+INTE_FILE_ICON_HEIGHT*i;
				//  fill_rect(buf, weight, x, y, 62, 55, DARK_GRAY);
                creat_file_icon(buf, weight, x, y);
                
                if(strlen(file_name_list[num])>7)
                {
                    file_name_list[num][5] = '.';
                    file_name_list[num][6] = '.';
                    file_name_list[num][7] = '\0';
                }
                text_length = strlen(file_name_list[num]) * CHAR_W / 2;
				put_string(buf,weight,x+31-text_length,y+40,file_name_list[num],BLACK);
				// fill_rect(buf, weight, 8+x, 15+y, 45, 26, DARK_GRAY);
				num++;
				if(num>=length)
				{
					return;
				}
		}

	}
}