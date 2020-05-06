#include"temporaldatam.h"
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/time.h>
/*
		[[[[[[[		←	pfirst队首（入）
		[[[[[[[
		[[[[[[[
		[[[[[[[
		[[[[[[[
		[[[[[[[		←	pend队尾（出）
*/

void temporaldatam_v2::pushCounter(const vector<Counter> & vc)
{
	hz_cur = (hz_cur + 1) % Hz;
	if (hz_cur != 0)
		return;

    for (auto &c : vc)		//循环添加可疑轮廓集合中的每一个轮廓
    {
        int dis = 99999;
        Que *ptrq = nullptr;
        for (auto &q : que)
        {
            int d = distense_label(q.label, cul_label(c));
            if (d < dis)
            {
                dis = d;
                ptrq = &q;
            }
        }
        if (ptrq == nullptr || dis > Distense*Distense)	//	如果无法把轮廓归队	创建新队列
        {
            Que newq;

            newq.counters.push(c);
            newq.timestamp.push(timestamp_now);
            if (ISF(AREA))
                newq.area.push(f_area(c));
            if (ISF(PERIMETER))
                newq.perimeter.push(f_perimeter(c));
            if (ISF(CIRCLE))
                newq.circle.push(f_circle(c));
            if (ISF(NUM))
                newq.num.push(f_num(c));
            if (ISF(COC))
                newq.cofc.push(f_cofc(c));
            newq.label = cul_label(c);

            que.push_back(newq);
        }
        else							//	如果可以把轮廓归队	归队
        {
            ptrq->counters.push(c);
            ptrq->timestamp.push(timestamp_now);
            if (ISF(AREA))
                ptrq->area.push(f_area(c));
            if (ISF(PERIMETER))
                ptrq->perimeter.push(f_perimeter(c));
            if (ISF(CIRCLE))
                ptrq->circle.push(f_circle(c));
            if (ISF(NUM))
                ptrq->num.push(f_num(c));
            if (ISF(COC))
                ptrq->cofc.push(f_cofc(c));
            ptrq->label = ptrq->label*0.34 + cul_label(c)*0.66;
        }
    }
    update();
}

int temporaldatam_v2::update()
{
    int mode = 0;
    timestamp_now++;

    //	如果队列太长了就pop
    for (auto &q : que)
    {
        if (q.timestamp.front() < timestamp_now - qlength)
        {
            q.counters.pop();
            q.timestamp.pop();
            if (ISF(AREA))
                q.area.pop();
            if (ISF(PERIMETER))
                q.perimeter.pop();
            if (ISF(CIRCLE))
                q.circle.pop();
            if (ISF(NUM))
                q.num.pop();
            if (ISF(COC))
                q.cofc.pop();
        }
    }

    //	如果队列为空就删除
    auto p = que.begin();
    while (p != que.end())
    {
        if (p->counters.empty())
        {
            p = que.erase(p);
            continue;
        }
        p++;
    }

    //	如果两个队列太近了就删除元素少的
    for (int n = 0;n < que.size();n++)
        for (int m = n + 1; m < que.size();m++)
            if (distense_label(que[n].label, que[m].label) <= Distense*Distense) {
                cout << "发生碰撞" << endl;
                if (que[n].counters.size() > que[m].counters.size()) {
                    que.erase(que.begin() + m);
                    m--;
                }
                else {
                    que.erase(que.begin() + n);
                    n--;
                    break;
                }
            }


    return mode;
}

temporaldatam_v2::Label temporaldatam_v2::cul_label(const Counter &p) const
{
    int sumx = 0;
    int sumy = 0;
    for (Point i : p)
    {
        sumx += i.x;
        sumy += i.y;
    }
    return Point(sumx / p.size(), sumy / p.size());
}


int remove_dir(const char* dir)
{
	char cur_dir[] = ".";
	char up_dir[] = "..";
	char dir_name[128];
	DIR* dirp;
	struct dirent* dp;
	struct stat dir_stat;

	// 参数传递进来的目录不存在，直接返回
	if (0 != access(dir, F_OK)) {
		return 0;
	}

	// 获取目录属性失败，返回错误
	if (0 > stat(dir, &dir_stat)) {
		perror("get directory stat error");
		return -1;
	}

	if (S_ISREG(dir_stat.st_mode)) {	// 普通文件直接删除
		remove(dir);
	}
	else if (S_ISDIR(dir_stat.st_mode)) {	// 目录文件，递归删除目录中内容
		dirp = opendir(dir);
		while ((dp = readdir(dirp)) != NULL) {
			// 忽略 . 和 ..
			if ((0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name))) {
				continue;
			}

			sprintf(dir_name, "%s/%s", dir, dp->d_name);
			remove_dir(dir_name);   // 递归调用
		}
		closedir(dirp);

		rmdir(dir);		// 删除空目录
	}
	else {
		perror("unknow file type!");
	}

	return 0;
}

struct timeval send_wait;
struct timeval send_end;

void alarm_ctrl::update()
{

    if(alarm_time.isempty())
        return;
    State_mes now;
    now.settime_now();
    record_time cur_time(now.year,now.mon,now.day,now.hour,now.min,now.sec-wait_time);
    //cout<<"get alarm_time: "<<alarm_time.hour<<"    "<<alarm_time.min<<"    "<<alarm_time.sec<<endl;
    //cout<<"set cur_time: "<<cur_time.hour<<"    "<<cur_time.min<<"    "<<cur_time.sec<<endl;

    if(cur_time<=alarm_time)
        return;
    State_mes mes;
    mes.getfromfile(mesname);
    record_time begin(mes.year, mes.mon, mes.day, mes.hour, mes.min, mes.sec - 1);
    record_time end(mes.year, mes.mon, mes.day, mes.hour, mes.min, mes.sec + 1);
    cout<<"record "<<begin.hour<<"  "<<begin.min<<"   "<<begin.sec<<" to "<<end.hour<<"  "<<end.min<<"   "<<end.sec<<endl;
    mycapture->Vedio_record(begin, end, port, vedioname);

	server->send_pkg(mes, rgbname, irname, uvname, vedioname);
	alarm_time.clear();

    gettimeofday(&send_end, NULL );
    double diff=(send_end.tv_sec - send_wait.tv_sec ) + (double)(send_end.tv_usec -send_wait.tv_usec)/1000000;
    cout<<"use time: "<<diff<<endl;
}

void alarm_ctrl::save_and_send(State_mes mes,Mat rgb,Mat ir,Mat uv,string basef, int pt)
{
	record_time cur_time;
	cur_time.settimenow();

    if(cur_time < stay_time || !alarm_time.isempty())
        return;

    gettimeofday(&send_wait, NULL );

	basefile = basef;
	alarm_time.set(mes.year, mes.mon, mes.day, mes.hour, mes.min, mes.sec);
    string filename=mes.tostring();
    string command = "mkdir -p " + basefile + filename;
    system(command.c_str());

    mesname = basefile + filename+"/"+filename+"_mes.txt";
    rgbname = basefile + filename+"/"+filename+"_rgb.jpg";
    irname = basefile + filename+"/"+filename+"_ir.jpg";
    uvname = basefile + filename+"/"+filename+"_uv.jpg";
    vedioname = basefile + filename+"/"+filename+"_vedio.mp4";
    port=pt;

    mes.save(mesname);
    imwrite(rgbname,rgb);
    imwrite(irname,ir);
    imwrite(uvname,uv);
}


void alarm_ctrl::clear()
{
	if (alarm_time.isempty())
		return;
	State_mes mes;
	mes.getfromfile(mesname);
	string filename = basefile + mes.tostring();
	remove_dir(filename.c_str());

	alarm_time.clear();
	basefile.clear();
	mesname.clear();
	rgbname.clear();
	irname.clear();
	uvname.clear();
	vedioname.clear();
    port=0;
}

void alarm_ctrl::setstaytime(int t)
{
	State_mes now;
	now.settime_now();
	stay_time.set(now.year, now.mon, now.day, now.hour, now.min, now.sec + t);
}

bool alarm_ctrl::isstay()
{
	if (stay_time.isempty())
		return false;
	else
	{
		record_time cur_time;
		cur_time.settimenow();
		if (cur_time < stay_time)
			return true;
		else
			return false;
	}
}
