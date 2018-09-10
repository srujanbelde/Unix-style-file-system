#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>

typedef struct Student{
	int rollno;
	char name[24];
	char college[20];
} student; //48

typedef struct TeachingStaff{
	int staffid;
	char name[24];
	char dept[12];
	char college[24];
} teachingstaff; //64

typedef struct TLeafPage{
	int pagetype;
	int tableId;
	teachingstaff data[7];
	char unused[48];
	int index;
	int previous;
} tleafpage; //512

typedef struct LeafPage{
	int pagetype;
	int tableId;
	student data[10];
	char unused[20];
	int index;
} leafpage; //512

typedef struct NonLeafPage{
	int pagetype;
	int keys[63];
	int offsets[64];
} nonleafpage; //512

typedef struct TableSector{
	int tableIds[8];
	int rootPageIds[8];
} tablesector; //64

void PrintStudent(student stud)
{
	printf("%d %s %s\n", stud.rollno, stud.name, stud.college);
}

void PrintTeacher(teachingstaff stud)
{
	printf("%d %s %s %s\n", stud.staffid, stud.name, stud.college, stud.dept);
}

void BuildBtree(char* csv, char* bin)
{
	FILE* fp = fopen(csv, "rb+");
	FILE* fw = fopen(bin, "wb+");
	int i, j, lpo = 0;

	nonleafpage nlpage;
	memset(&nlpage, 0, sizeof(nonleafpage));
	fwrite(&nlpage, sizeof(nonleafpage), 1, fw);

	nlpage.pagetype = 2;
	j = 0;
	while (!feof(fp))
	{
		leafpage lpage;
		memset(&lpage, 0, sizeof(leafpage));

		lpage.pagetype = 1;

		for (i = 0; i < 10; i++)
		{
			fscanf(fp, "%d,%[^,],%[^\n]\n", &lpage.data[i].rollno, &lpage.data[i].name, &lpage.data[i].college);
			//PrintStudent(lpage.data[i]);
		}

		nlpage.offsets[lpo] = ftell(fw);
		printf("%d ", ftell(fw));
		fwrite(&lpage, sizeof(leafpage), 1, fw);
		lpo++;

		if (j > 0){
			nlpage.keys[j-1] = lpage.data[0].rollno;
			printf("%d ", lpage.data[0].rollno);
		}
		printf("\n");

		j++;
	}

	fseek(fw, 0, SEEK_SET);
	fwrite(&nlpage, sizeof(nonleafpage), 1, fw);

	fclose(fp);
	fclose(fw);

}

void BuildBtree2(char* csv, char* bin)
{
	FILE* fp = fopen(csv, "rb+");
	FILE* fw = fopen(bin, "wb+");
	FILE* ft;
	int i, j, lpo = 0;
	int recordCount = 0;

	fseek(fw, sizeof(tablesector), SEEK_SET); //first iteration: leaving 64 bytes

	tablesector tsec;
	memset(&tsec, 0, sizeof(tsec));

	tsec.rootPageIds[0] = 0; //offset of current non leaf page

	nonleafpage nlpage;
	memset(&nlpage, 0, sizeof(nonleafpage));
	nlpage.pagetype = 2;

	while (!feof(fp))
	{

		j = 0;
		while (j<=recordCount)
		{
			lpo = 0;
			leafpage lpage;
			memset(&lpage, 0, sizeof(leafpage));

			lpage.pagetype = 1;

			for (i = 0; i < 10; i++)
			{
				fscanf(fp, "%d,%[^,],%[^\n]\n", &lpage.data[i].rollno, &lpage.data[i].name, &lpage.data[i].college);
				PrintStudent(lpage.data[i]);
				lpage.index = i;
				recordCount++;
				if (feof(fp))
					break;
			}

			nlpage.offsets[lpo] = ftell(fw);
			printf("%d ", ftell(fw));
			fwrite(&lpage, sizeof(leafpage), 1, fw);
			lpo++;

			if (j > 0){
				nlpage.keys[j - 1] = lpage.data[0].rollno;
				printf("%d ", lpage.data[0].rollno);
			}
			printf("\n");

			tsec.rootPageIds[0] = ftell(fw);
			fwrite(&nlpage, sizeof(nonleafpage), 1, fw);

			//update the table sector
			ft = fopen(bin, "rb+");
			fwrite(&tsec, sizeof(tablesector), 1, ft);
			fclose(ft);

			j++;

			if (feof(fp)) break;
		}

	}

	fclose(fp);
	fclose(fw);
}

void BuildBTree3(char* csv, char* tcsv, char* bin)
{
	FILE* fp = fopen(csv, "rb+");
	FILE* fpt = fopen(tcsv, "rb+");
	FILE* fw = fopen(bin, "wb+");
	FILE* ft;
	int i, j, lpo = 0;
	int recordCount = 0, trecordCount = 0;

	fseek(fw, sizeof(tablesector), SEEK_SET); //first iteration: leaving 64 bytes

	tablesector tsec;
	memset(&tsec, 0, sizeof(tsec));

	tsec.rootPageIds[0] = 0; //offset of current non leaf page
	tsec.tableIds[0] = 1; //students
	tsec.tableIds[1] = 2; //teachers

	nonleafpage nlpage, tnlpage;
	memset(&nlpage, 0, sizeof(nonleafpage));
	nlpage.pagetype = 2;

	memset(&tnlpage, 0, sizeof(nonleafpage));
	tnlpage.pagetype = 2;

	lpo = 0; //no of total pages
	int prev = -1;

	while (!feof(fp) && !feof(fpt))
	{
		j = 0;
		while (j <= recordCount && j <= trecordCount)
		{
			leafpage lpage;
			memset(&lpage, 0, sizeof(leafpage));
			lpage.tableId = 1;
			lpage.pagetype = 1;

			tleafpage tlpage;
			memset(&tlpage, 0, sizeof(tleafpage));
			tlpage.tableId = 2;
			tlpage.pagetype = 1;

			for (i = 0; i < 10; i++)
			{
				fscanf(fp, "%d,%[^,],%[^\n]\n", &lpage.data[i].rollno, &lpage.data[i].name, &lpage.data[i].college);
				//PrintStudent(lpage.data[i]);
				lpage.index = i;
				recordCount++;
				if (feof(fp))
					break;
			}

			nlpage.offsets[j] = ftell(fw);
			printf("%d ", ftell(fw));
			fwrite(&lpage, sizeof(leafpage), 1, fw);
			lpo++;

			if (j > 0){
				nlpage.keys[j - 1] = lpage.data[0].rollno;
				printf("%d ", lpage.data[0].rollno);
			}
			printf("\n");

			tsec.rootPageIds[0] = ftell(fw);
			fwrite(&nlpage, sizeof(nonleafpage), 1, fw);

			//teachers
			for (i = 0; i < 7; i++)
			{
				fscanf(fpt, "%d,%[^,],%[^,],%[^\n]\n", &tlpage.data[i].staffid, &tlpage.data[i].name, &tlpage.data[i].college, &tlpage.data[i].dept);
				PrintTeacher(tlpage.data[i]);
				tlpage.index = i;
				trecordCount++;
				if (feof(fpt))
					break;
			}

			tnlpage.offsets[j] = ftell(fw);
			printf("%d ", ftell(fw));
			tlpage.previous = prev;
			prev = ftell(fw);
			fwrite(&tlpage, sizeof(tleafpage), 1, fw);
			lpo++;

			if (j > 0){
				tnlpage.keys[j - 1] = tlpage.data[0].staffid;
				printf("%d ", tlpage.data[0].staffid);
			}
			printf("\n");

			tsec.rootPageIds[1] = ftell(fw);
			fwrite(&tnlpage, sizeof(nonleafpage), 1, fw);

			//update the table sector
			ft = fopen(bin, "rb+");
			fwrite(&tsec, sizeof(tablesector), 1, ft);
			fclose(ft);

			j++;

			if (feof(fp) || feof(fpt)) break;
		}

	}

	fclose(fp);
	fclose(fw);
}

int isEqual(char *a, char *b)
{
	int i;
	for (i = 0; a[i] != '\r', b[i] != '\r', a[i] != '\0'; i++)
		if (a[i] != b[i])
			return 0;

	return 1;
}

int MatchingRecords(int rollno, char* bin)
{
	//given student id, find teachers in his college
	FILE* fp = fopen(bin, "rb+");
	int i, teachers = 0;
	char college[20] = { '\0' };

	tablesector tsec;
	memset(&tsec, 0, sizeof(tablesector));
	fread(&tsec, sizeof(tablesector), 1, fp);

	//search student tree
	int soffset = tsec.rootPageIds[0];
	nonleafpage nlp;
	memset(&nlp, 0, sizeof(nonleafpage));
	fseek(fp, soffset, SEEK_SET);
	fread(&nlp, sizeof(nonleafpage), 1, fp);

	for (i = 0; i < 63; i++)
	{
		//printf("%d\n", nlp.keys[i]);
		if (rollno <= nlp.keys[i]) //< works
			break;
	}

	leafpage lp;
	memset(&lp, 0, sizeof(leafpage));
	fseek(fp, nlp.offsets[i], SEEK_SET);
	fread(&lp, sizeof(leafpage), 1, fp);

	for (i = 0; i < 10; i++)
	{
		//PrintStudent(lp.data[i]);
		if (lp.data[i].rollno == rollno)
			break;
	}

	char name[20] = { '\0' };
	strcpy(name, lp.data[i].name);
	strcpy(college, lp.data[i].college);
	printf("%s\n", college);

	//search teacher tree
	memset(&nlp, 0, sizeof(nonleafpage));
	fseek(fp, tsec.rootPageIds[1], SEEK_SET);
	fread(&nlp, sizeof(nonleafpage), 1, fp);

	tleafpage tlp;
	memset(&tlp, 0, sizeof(tleafpage));
	fseek(fp, nlp.offsets[63], SEEK_SET);

	do
	{
		memset(&tlp, 0, sizeof(tleafpage));
		fread(&tlp, sizeof(tleafpage), 1, fp);
		for (i = 0; i < 7; i++)
		{
			//PrintTeacher(tlp.data[i]);
			//printf("%d\n", isEqual(tlp.data[i].college, college));
			if (isEqual(tlp.data[i].college, college))
				teachers++;
		}
		
		fseek(fp, tlp.previous, SEEK_SET);
	} while (tlp.previous != -1);
	

	printf("%d teachers are teaching %s.", teachers, name);
}

main()
{
	//printf("%d %d %d %d", sizeof(teachingstaff), sizeof(tleafpage), sizeof(nonleafpage), sizeof(tablesector));
	//BuildBTree3("640studs.csv", "teacherdata.csv", "store.bin");

	MatchingRecords(68, "store.bin");

	_getch();
}
