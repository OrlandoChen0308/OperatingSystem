#include "frame_pool.H"

FramePool* FramePool::pools[2];//One for kernel memory pool and one for process memory pool
int FramePool::numberOfFramePools = 0;

FramePool::FramePool(unsigned long _base_frame_no,
             unsigned long _nframes,
             unsigned long _info_frame_no){

	base_frame_no = _base_frame_no;
	nframes = _nframes;
	info_frame_no = _info_frame_no;

	if (_info_frame_no == 0){
		bitmap = (unsigned char*)(((2 MB)/(4 KB))*FRAME_SIZE);
		bitmap[0] = 1;
	}
	else{
		bitmap = _info_frame_no*FRAME_SIZE;
	}

	for (unsigned long i = 1; i < nframes/8; i++){
		bitmap[i] = 0;
	}
  	pools[numberOfFramePools] = this;
	numberOfFramePools++;  
}

	unsigned long FramePool::get_frame(){
		unsigned long i;

		for (i = 0; i < nframes/8; i++){
			if ( bitmap[i]!= 0xFF ){
				break;	
			}
		}
        if( i == nframes/8){
        	return 0;
		}else{
			unsigned long j = 0;
			unsigned char val = bitmap[i];
			while(true){
				if ((val & (0x01 << j)) == 0) {
					break;
				}
				j++;
			}
			bitmap[i] |= 0x01<<j;
			return (i*8+j)+base_frame_no;
		}
  }

	void FramePool::mark_inaccessible(unsigned long _base_frame_no,
                          					unsigned long _nframes){
		unsigned long position = _base_frame_no - base_frame_no;
		for (unsigned long i = 0; i < _nframes; i++){
			*(bitmap+(position+i)/8) |= (1 << ((position+i)%8));
		}
	}

  void FramePool::release_frame(unsigned long _frame_no){
		unsigned long current_frame_no;
		unsigned long current_nframe;
		int i;
		for(i = 0; i<numberOfFramePools; i++){
			current_frame_no = pools[i]->base_frame_no;
			current_nframe = pools[i]->nframes;
			if((_frame_no<=current_frame_no+current_nframe)&&(_frame_no>=current_frame_no)){
				break;
			}
		}

    unsigned long actual_position=_frame_no - pools[i]->base_frame_no;
    unsigned char mask=0x00;
    unsigned int offset=actual_position%8;
    mask |=(0x01<<offset);
    mask = ~mask;
    *(pools[i]->bitmap+(actual_position/8)) &= mask;
	}
