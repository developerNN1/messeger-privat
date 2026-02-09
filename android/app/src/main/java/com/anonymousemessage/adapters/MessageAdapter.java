package com.anonymousemessage.adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.anonymousemessage.R;
import com.anonymousemessage.models.Message;
import java.text.SimpleDateFormat;
import java.util.List;
import java.util.Locale;

public class MessageAdapter extends ArrayAdapter<Message> {

    private static final int VIEW_TYPE_OUTGOING = 0;
    private static final int VIEW_TYPE_INCOMING = 1;
    private static final int VIEW_TYPE_COUNT = 2;

    private final LayoutInflater inflater;
    private final SimpleDateFormat dateFormat;

    public MessageAdapter(@NonNull Context context, @NonNull List<Message> objects) {
        super(context, 0, objects);
        this.inflater = LayoutInflater.from(context);
        this.dateFormat = new SimpleDateFormat("HH:mm", Locale.getDefault());
    }

    @Override
    public int getViewTypeCount() {
        return VIEW_TYPE_COUNT;
    }

    @Override
    public int getItemViewType(int position) {
        Message message = getItem(position);
        // This is a simplified check - in real implementation, you'd compare senderId with current user ID
        return (message.getSenderId().equals("current_user_id")) ? VIEW_TYPE_OUTGOING : VIEW_TYPE_INCOMING;
    }

    @NonNull
    @Override
    public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
        int viewType = getItemViewType(position);
        ViewHolder holder;

        if (convertView == null) {
            // Inflate the appropriate layout based on message direction
            int layoutId = (viewType == VIEW_TYPE_OUTGOING) ? 
                R.layout.item_message_outgoing : R.layout.item_message_incoming;
            convertView = inflater.inflate(layoutId, parent, false);
            
            holder = new ViewHolder();
            holder.messageText = convertView.findViewById(R.id.message_text);
            holder.messageTime = convertView.findViewById(R.id.message_time);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        Message message = getItem(position);
        if (message != null) {
            holder.messageText.setText(message.getContent());
            holder.messageTime.setText(dateFormat.format(message.getTimestamp()));
        }

        return convertView;
    }

    static class ViewHolder {
        TextView messageText;
        TextView messageTime;
    }
}