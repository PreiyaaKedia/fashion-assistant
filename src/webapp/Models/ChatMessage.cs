using System;
using System.Collections.Generic;

namespace dotnetfashionassistant.Models
{
    /// <summary>
    /// Represents a message in the chat interface.
    /// </summary>
    public class ChatMessage
    {
        /// <summary>
        /// Gets or sets the content of the message.
        /// </summary>
        public string Content { get; set; } = string.Empty;
        
        /// <summary>
        /// Gets or sets the formatted HTML content of the message.
        /// This preserves formatting when navigating away and returning.
        /// </summary>
        public string FormattedContent { get; set; } = string.Empty;
        
        /// <summary>
        /// Gets or sets a value indicating whether the message is from the user (true) or the AI (false).
        /// </summary>
        public bool IsUser { get; set; }
        
        /// <summary>
        /// Gets or sets the timestamp when the message was created.
        /// </summary>
        public DateTime Timestamp { get; set; }
        
        /// <summary>
        /// Gets or sets the list of product IDs associated with this message.
        /// Used for displaying product images when relevant.
        /// </summary>
        public List<int> ProductIds { get; set; } = new List<int>();
        
        /// <summary>
        /// Gets or sets a value indicating whether the message should show product images.
        /// </summary>
        public bool ShowProductImages { get; set; } = false;
    }
}