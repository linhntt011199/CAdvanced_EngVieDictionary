#include<gtk/gtk.h>
#include<string.h>
#include"btree.h"
#include"soundex.h"

GtkWidget *textView, *window;
GtkWidget *textSearch;
BTA *data, *soundexTree;
GtkTextBuffer *buffer;
GtkListStore *suggestList;

void destroy_c(GtkWidget *widget, gpointer gp){
	gtk_widget_destroy((GtkWidget *)gp);
}
int prefix(char *s1, char *s2){
	int len1=strlen(s1);
	int len2=strlen(s2),i;
	if(len1>len2) return 0;
	else{
		for(i=0;i<len1;i++)
			if(s1[i]!=s2[i]) return 0;
	}
	return 1;
}

char suggestion[15][100];
void suggest_function(GtkWidget* widget, gpointer soundexTree){
	int rsize, i=0;
	char en[100],soundex[5], word[50], tmp[1000], meaning[10000];
	strcpy(tmp, "\n");
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	if(gtk_entry_get_text_length(GTK_ENTRY(textSearch)) > 0){
			btsel(soundexTree,"", soundex,5*sizeof(char),&rsize);
			while(btseln(soundexTree, en, soundex, 5*sizeof(char),&rsize)==0 && i<15){
				if(prefix(word, en)==1){				
						strcpy(suggestion[i],en);
						strcat(tmp, suggestion[i]);
						strcat(tmp, "\n");
						i++;				
				}
			}
			tmp[strlen(tmp)-1] = '\0';
			gtk_text_buffer_set_text(buffer, tmp, -1);		
	}
	else gtk_text_buffer_set_text(buffer, "\n", -1);
}


void auto_complete(GtkWidget* widget, gpointer soundexTree){
	gtk_entry_set_text(GTK_ENTRY(textSearch), suggestion[0]);
	//display_meaning(widget, soundexTree);
}


void display_meaning(GtkWidget *widget, gpointer soundexTree){
	int rsize, beExisted, i;
	char word[50], meaning[5000], suggest[100][100];
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	beExisted = btsel(data, word, meaning, 5000, &rsize);
	if(beExisted == 0) gtk_text_buffer_set_text(buffer, meaning, -1);
	else gtk_text_buffer_set_text(buffer, "This word isn't found", -1);	
}

typedef struct{
		GtkWidget * data_array[3];
		BTA *tree;
	}data_struct;
void Show_message(GtkWidget * parent , GtkMessageType type,  char * mms, char * content) {
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT, type, GTK_BUTTONS_OK, "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}

void add_new_word(GtkWidget *widget, gpointer pointer){
	char word[50], meantext[10000], soundex[5];
	int rsize, insertDictResult, insertSoundexResult;
	data_struct *pt = (data_struct*)pointer;
	GtkWidget *inputtext = (GtkWidget**)(pt->data_array[0]);
	GtkWidget *mean = (GtkWidget**)(pt->data_array[1]);
	GtkWidget *dialog = (GtkWidget**)(pt->data_array[2]);
	BTA *soundexTree = (BTA**)(pt->tree);
	GtkTextIter st_iter;
	GtkTextIter ed_iter;
	gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter);
	gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &ed_iter);
	gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE);

	strcpy(meantext, gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE));
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(inputtext)));

	if(word[0] == '\0' || meantext[0] == '\0') Show_message(dialog, GTK_MESSAGE_WARNING, "An error occurred", "");
	else if(btsel(data, word, meantext, 5000, &rsize) == 0) Show_message(dialog, GTK_MESSAGE_WARNING, "This word is existed.","");
	else{
		insertDictResult = btins(data, word, meantext, strlen(meantext) + 1);
		SoundEx(soundex, word, 4, 1);
    		insertSoundexResult = btins(soundexTree, word, soundex, 5*sizeof(char));
		if(insertDictResult == 0 && insertSoundexResult == 0) Show_message(dialog, GTK_MESSAGE_INFO, "Successful!","");
		else Show_message(dialog, GTK_MESSAGE_INFO, "Unsuccessful","");
	}
	
}
void show_add_dialog(GtkWidget *widget, gpointer soundexTree){
	GtkWidget *adddialog;
	adddialog = gtk_dialog_new_with_buttons("Add word", GTK_WINDOW(window),GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();

	GtkWidget* tframe = gtk_frame_new("Word:");
	gtk_widget_set_size_request(tframe, 300, 50);
	GtkWidget* inputtext = gtk_entry_new();
	gtk_widget_set_margin_left(inputtext, 2);
	gtk_widget_set_margin_right(inputtext, 2);
	gtk_container_add(GTK_CONTAINER(tframe), inputtext);
	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 0);

	GtkWidget* bframe = gtk_frame_new("Mean:");
	gtk_widget_set_size_request(bframe, 300, 200);
	GtkWidget* mean = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(mean), GTK_WRAP_WORD_CHAR);		//Chong tran be ngang
	GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 55);
	gtk_container_add(GTK_CONTAINER(scroll), mean);
	gtk_container_add(GTK_CONTAINER(bframe), scroll);

	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget * OkButton =  gtk_button_new_with_label("Add");
	GtkWidget * CancelButton = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(box), OkButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), CancelButton, TRUE, TRUE, 2);
	gtk_widget_set_size_request(box, 300, 50);
	gtk_widget_set_size_request(OkButton, 100, 40);
	gtk_widget_set_size_request(CancelButton, 100, 40);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 260);
	
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(adddialog))), dialog_ground,  TRUE, TRUE, 0);
	
	
	data_struct pointer;
	pointer.data_array[0] = inputtext;
	pointer.data_array[1] = mean;
	pointer.data_array[2] = adddialog;
	pointer.tree = soundexTree;
	g_signal_connect(OkButton, "clicked", G_CALLBACK(add_new_word), &pointer);
	g_signal_connect(CancelButton, "clicked", G_CALLBACK(destroy_c), adddialog);

	gtk_widget_show_all(adddialog);
	gtk_dialog_run(GTK_DIALOG(adddialog));
	gtk_widget_destroy(adddialog);
}

void edit_word(GtkWidget *widget, gpointer pointer){
	data_struct* pt = (data_struct*)pointer;
	GtkWidget *inputtext = (GtkWidget**)(pt->data_array[0]);
	GtkWidget *mean = (GtkWidget**)(pt->data_array[1]);
	GtkWidget *dialog = (GtkWidget**)(pt->data_array[2]);
	BTA *soundexTree = (BTA**)(pt->tree);
	GtkTextIter st_iter;
	GtkTextIter ed_iter;
	int result, rsize;
	char word[50], meantext[10000];
	gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter);
	gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &ed_iter);

	strcpy(word, gtk_entry_get_text(GTK_ENTRY(inputtext)));
	
	
	if (word[0] == '\0' || meantext[0] == '\0') Show_message(dialog, GTK_MESSAGE_WARNING, "An error occurred","");
	else if(btsel(data, word, meantext, 5000, &rsize) != 0) Show_message(dialog, GTK_MESSAGE_ERROR, "This word isn't existed","");
	else {
		strcpy(meantext, gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE));
		result = btupd(data, word, meantext, strlen(meantext) + 1);
		
		if (result == 0)Show_message(dialog, GTK_MESSAGE_INFO, "Successful","");
		else Show_message(dialog, GTK_MESSAGE_ERROR, "Unsuccessful", "");
	}
}

void show_edit_dialog(GtkWidget *widget, gpointer soundexTree){
	int rsize;
	char word[50], meantext[10000];
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	if (gtk_entry_get_text_length(GTK_ENTRY(textSearch)) == 0 ||
	        btsel(data, word, meantext, 5000, &rsize) != 0) {
		Show_message(window, GTK_MESSAGE_WARNING, "This word isn't existed", "");
		return;
	}
	GtkWidget *editdialog;
	editdialog = gtk_dialog_new_with_buttons("Edit word", GTK_WINDOW(window),GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();
	
	GtkWidget* tframe = gtk_frame_new("Word");
	gtk_widget_set_size_request(tframe, 300, 50);
	GtkWidget* inputtext = gtk_search_entry_new();
	gtk_widget_set_margin_left(inputtext, 2);
	gtk_widget_set_margin_right(inputtext, 2);
	gtk_container_add(GTK_CONTAINER(tframe), inputtext);
	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 0);

	
	GtkWidget* bframe = gtk_frame_new("Mean");
	gtk_widget_set_size_request(bframe, 300, 200);
	GtkWidget* mean = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(mean), GTK_WRAP_WORD_CHAR);//Chong tran be ngang
	GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll), mean);
	gtk_container_add(GTK_CONTAINER(bframe), scroll);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 55);

	
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_size_request(box, 300, 50);
	GtkWidget * OkButton =  gtk_button_new_with_label("Save");
	GtkWidget * CancelButton = gtk_button_new_with_label("Cancel");
	gtk_widget_set_size_request(OkButton, 100, 40);
	gtk_widget_set_size_request(CancelButton, 100, 40);
	gtk_box_pack_start(GTK_BOX(box), OkButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), CancelButton, TRUE, TRUE, 2);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 260);

	
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(editdialog))), dialog_ground,  TRUE, TRUE, 0);
	gtk_widget_set_sensitive(tframe, FALSE);
	gtk_entry_set_text(GTK_ENTRY(inputtext), gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(mean), gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView)));


	data_struct pointer;
	pointer.data_array[0] = inputtext;
	pointer.data_array[1] = mean;
	pointer.data_array[2] = editdialog;
	pointer.tree = soundexTree;
	g_signal_connect(OkButton, "clicked", G_CALLBACK(edit_word), &pointer);
	g_signal_connect(CancelButton, "clicked", G_CALLBACK(destroy_c), editdialog);

	gtk_widget_show_all(editdialog);
	gtk_dialog_run(GTK_DIALOG(editdialog));
	gtk_widget_destroy(editdialog);

}


void delete_word_from_dict(char *word, BTA* soundexTree){
	int delInDictResult, delInSoundexResult;
	char a[100] = "Delete this word";
	delInDictResult = btdel(data, word);
	delInSoundexResult = btdel(soundexTree, word);
	if (delInDictResult == 0 && delInSoundexResult == 0)Show_message(window, GTK_MESSAGE_INFO, "Successful","");
	else Show_message(window, GTK_MESSAGE_ERROR, "An error occurred", "");
	gtk_entry_set_text(GTK_ENTRY(textSearch), "");
}

void show_delete_dialog(GtkWidget *widget, gpointer soundexTree){
	char word[50], meantext[10000];
	int result, rsize;
	GtkWidget *deldialog;
	BTA* tree = (BTA*)soundexTree;

	strcpy(word, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	if (gtk_entry_get_text_length(GTK_ENTRY(textSearch)) == 0 || btsel(data, word, meantext, 5000, &rsize) != 0) {
		Show_message(window, GTK_MESSAGE_WARNING, "This word isn't existed", "");
		return;
	}
	
	deldialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,
	                                   "Delete: \"%s\"?", gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(deldialog), "Do you want delete \"%s\" ?",
							gtk_entry_get_text(GTK_ENTRY(textSearch)));

	result = gtk_dialog_run(GTK_DIALOG(deldialog));
	if (result == GTK_RESPONSE_YES)delete_word_from_dict(word, tree);
	gtk_widget_destroy(deldialog);
}

void separate_mean(char* mean) {
  int i = 0, j = 1;
  while (mean[j] != '\0') {
    if (mean[j] == '\\' && mean[j + 1] == 'n') {
      mean[i++] = '\n';
      j += 2;
    }
    else {
      if (i != j)mean[i++] = mean[j++];
      else {
        i++; j++;
      }
    }
  }
  mean[i] = '\0';
}
